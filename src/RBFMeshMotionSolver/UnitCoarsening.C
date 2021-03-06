
/*
 * Author
 *   David Blom, TU Delft. All rights reserved.
 */

#include "UnitCoarsening.H"

namespace rbf
{
    UnitCoarsening::UnitCoarsening(
        double tol,
        int minPoints,
        int maxPoints
        )
        :
        tol( tol ),
        minPoints( minPoints ),
        maxPoints( maxPoints )
    {
        assert( maxPoints >= minPoints );
        assert( tol > 0 );
        assert( tol <= 1 );
    }

    UnitCoarsening::~UnitCoarsening()
    {}

    void UnitCoarsening::compute(
        std::shared_ptr<RBFFunctionInterface> rbfFunction,
        std::unique_ptr<ElDistVector> positions,
        std::unique_ptr<ElDistVector> positionsInterpolation
        )
    {
        // selection is only performed once. Thereafter, the selected points are used for the
        // interpolation.

        selectedPositions.clear();

        // An initial selection is needed before the greedy algorithm starts
        // adding points to the selection.
        // The first point is the point with the largest radius from the origin.
        // The second point is the point with the largest distance from the
        // first point.

        {
            // Find first point: largest radius from origin
            ElDistVector norms( positions->Grid() );
            norms.AlignWith( *positions );
            El::RowTwoNorms( *positions, norms );
            El::Entry<double> locMax = El::MaxAbsLoc( norms );
            selectedPositions.push_back( locMax.i );

            // Find second point: largest distance from the first point
            ElDistVector distance = *positions;
            ElDistVector tmp( distance.Grid() );
            tmp.AlignWith( distance );
            El::Ones( tmp, distance.Height(), distance.Width() );

            for ( int iColumn = 0; iColumn < tmp.Width(); iColumn++ )
            {
                ElDistVector view( tmp.Grid() );
                El::View( view, tmp, 0, iColumn, tmp.Height(), 1 );
                El::Scale( positions->Get( locMax.i, iColumn ), view );
            }

            El::Axpy( -1, tmp, distance );

            El::RowTwoNorms( distance, norms );
            locMax = El::MaxAbsLoc( norms );
            selectedPositions.push_back( locMax.i );
        }

        int maxPoints = std::min( this->maxPoints, positions->Height() );
        int minPoints = std::min( this->minPoints, positions->Height() );

        double largestError = 0;

        for ( int i = 0; i < maxPoints; i++ )
        {
            // Build the matrices for the RBF interpolation
            std::unique_ptr<ElDistVector> positionsCoarse( new ElDistVector( positions->Grid() ) );
            std::unique_ptr<ElDistVector> valuesCoarse( new ElDistVector( positions->Grid() ) );
            positionsCoarse->AlignWith( *positions );
            valuesCoarse->AlignWith( *positions );
            El::Ones( *valuesCoarse, selectedPositions.size(), positions->Width() );
            El::Zeros( *positionsCoarse, selectedPositions.size(), positions->Width() );

            selectData( positions, positionsCoarse );

            // Perform the RBF interpolation
            std::unique_ptr<ElDistVector> positionsInterpolationCoarse( new ElDistVector( positions->Grid() ) );

            *positionsInterpolationCoarse = *positions;

            ElRBFInterpolation rbf( rbfFunction, std::move( positionsCoarse ), std::move( positionsInterpolationCoarse ) );

            std::unique_ptr<ElDistVector> result = rbf.interpolate( valuesCoarse );

            assert( result->Height() == positions->Height() );
            assert( result->Width() == positions->Width() );

            // Compute the error
            ElDistVector diff( result->Grid() );
            diff.AlignWith( *result );
            El::Ones( diff, result->Height(), result->Width() );
            El::Axpy( -1, *result, diff );
            ElDistVector errors( diff.Grid() );
            errors.AlignWith( diff );
            El::RowTwoNorms( diff, errors );

            // Get location of max error
            El::Entry<double> locMax = El::MaxAbsLoc( errors );
            largestError = locMax.value;

            // Break if maximum points are reached
            if ( int( selectedPositions.size() ) >= maxPoints )
                break;

            bool convergence = locMax.value < tol && int( selectedPositions.size() ) >= minPoints;

            if ( convergence )
                break;

            selectedPositions.push_back( locMax.i );
        }

        if ( El::mpi::Rank() == 0 )
        {
            std::cout << "RBF interpolation coarsening: selected ";
            std::cout << selectedPositions.size() << "/" << positions->Height() << " points,";
            std::cout << " error = " << largestError << ", tol = " << tol << std::endl;
        }

        std::unique_ptr<ElDistVector> positionsCoarse( new ElDistVector( positions->Grid() ) );
        positionsCoarse->AlignWith( *positions );
        El::Zeros( *positionsCoarse, selectedPositions.size(), positions->Width() );

        selectData( positions, positionsCoarse );

        rbf.compute( rbfFunction, std::move( positionsCoarse ), std::move( positionsInterpolation ) );
    }

    bool UnitCoarsening::initialized()
    {
        return rbf.initialized();
    }

    std::unique_ptr<ElDistVector> UnitCoarsening::interpolate( const std::unique_ptr<ElDistVector> & values )
    {
        std::unique_ptr<ElDistVector> selectedValues( new ElDistVector( values->Grid() ) );
        selectedValues->AlignWith( *values );
        El::Zeros( *selectedValues, selectedPositions.size(), values->Width() );

        selectData( values, selectedValues );

        return rbf.interpolate( selectedValues );
    }

    void UnitCoarsening::selectData(
        const std::unique_ptr<ElDistVector> & data,
        std::unique_ptr<ElDistVector> & selection
        )
    {
        assert( selection->Height() == int( selectedPositions.size() ) );

        int nbPulls = 0;

        for ( size_t j = 0; j < selectedPositions.size(); j++ )
        {
            for ( int iDim = 0; iDim < data->Width(); iDim++ )
            {
                if ( selection->Owner( j, iDim ) == El::mpi::Rank( selection->DistComm() ) )
                    nbPulls++;
            }
        }

        data->ReservePulls( nbPulls );

        for ( size_t j = 0; j < selectedPositions.size(); j++ )
        {
            for ( int iDim = 0; iDim < data->Width(); iDim++ )
            {
                if ( selection->Owner( j, iDim ) == El::mpi::Rank( selection->DistComm() ) )
                    data->QueuePull( selectedPositions[j], iDim );
            }
        }

        std::vector<double> buffer;
        data->ProcessPullQueue( buffer );
        int index = 0;

        for ( size_t j = 0; j < selectedPositions.size(); j++ )
        {
            for ( int iDim = 0; iDim < data->Width(); iDim++ )
            {
                if ( selection->Owner( j, iDim ) == El::mpi::Rank( selection->DistComm() ) )
                {
                    selection->Set( j, iDim, buffer[index] );
                    index++;
                }
            }
        }
    }
}
