
/*
 * Author
 *   David Blom, TU Delft. All rights reserved.
 */

#include <unordered_map>
#include "RBFCoarsening.H"
#include "WendlandC2Function.H"
#include "TPSFunction.H"

namespace rbf
{
    RBFCoarsening::RBFCoarsening()
        :
        rbf( std::shared_ptr<RBFInterpolation> ( new RBFInterpolation() ) ),
        rbfCoarse( std::shared_ptr<RBFInterpolation> ( new RBFInterpolation( rbf->rbfFunction, rbf->polynomialTerm, rbf->cpu ) ) ),
        enabled( false ),
        livePointSelection( false ),
        livePointSelectionSumValues( false ),
        tol( 0 ),
        tolLivePointSelection( 0 ),
        coarseningMinPoints( 0 ),
        coarseningMaxPoints( 0 ),
        twoPointSelection( false ),
        surfaceCorrection( false ),
        ratioRadiusError( 10 ),
        exportTxt( false ),
        selectedPositions(),
        nbStaticFaceCentersRemove( 0 ),
        positions(),
        positionsInterpolation(),
        values(),
        errorInterpolationCoarse(),
        closestBoundaryIndexCorrection(),
        valuesCorrection(),
        nbMovingFaceCenters( 0 ),
        fileExportIndex( 0 )
    {
        assert( rbf );
    }

    RBFCoarsening::RBFCoarsening( std::shared_ptr<RBFInterpolation> rbf )
        :
        rbf( rbf ),
        rbfCoarse( std::shared_ptr<RBFInterpolation> ( new RBFInterpolation( rbf->rbfFunction, rbf->polynomialTerm, rbf->cpu ) ) ),
        enabled( false ),
        livePointSelection( false ),
        livePointSelectionSumValues( false ),
        tol( 0 ),
        tolLivePointSelection( 0 ),
        coarseningMinPoints( 0 ),
        coarseningMaxPoints( 0 ),
        twoPointSelection( false ),
        surfaceCorrection( false ),
        ratioRadiusError( 10 ),
        exportTxt( false ),
        selectedPositions(),
        nbStaticFaceCentersRemove( 0 ),
        positions(),
        positionsInterpolation(),
        values(),
        errorInterpolationCoarse(),
        closestBoundaryIndexCorrection(),
        valuesCorrection(),
        nbMovingFaceCenters( 0 ),
        fileExportIndex( 0 )
    {
        assert( rbf );
    }

    RBFCoarsening::RBFCoarsening(
        std::shared_ptr<RBFInterpolation> rbf,
        bool enabled,
        bool livePointSelection,
        bool livePointSelectionSumValues,
        scalar tol,
        scalar tolLivePointSelection,
        int coarseningMinPoints,
        int coarseningMaxPoints,
        bool exportTxt
        )
        :
        rbf( rbf ),
        rbfCoarse( std::shared_ptr<RBFInterpolation> ( new RBFInterpolation( rbf->rbfFunction, rbf->polynomialTerm, rbf->cpu ) ) ),
        enabled( enabled ),
        livePointSelection( livePointSelection ),
        livePointSelectionSumValues( livePointSelectionSumValues ),
        tol( tol ),
        tolLivePointSelection( tolLivePointSelection ),
        coarseningMinPoints( coarseningMinPoints ),
        coarseningMaxPoints( coarseningMaxPoints ),
        twoPointSelection( false ),
        surfaceCorrection( false ),
        ratioRadiusError( 10.0 ),
        exportTxt( exportTxt ),
        selectedPositions(),
        nbStaticFaceCentersRemove( 0 ),
        positions(),
        positionsInterpolation(),
        values(),
        errorInterpolationCoarse(),
        closestBoundaryIndexCorrection(),
        valuesCorrection(),
        nbMovingFaceCenters( 0 ),
        fileExportIndex( 0 )
    {
        assert( rbf );
        assert( coarseningMinPoints <= coarseningMaxPoints );
        assert( coarseningMinPoints > 0 );
        assert( coarseningMaxPoints > 0 );
        assert( tol > 0 );
        assert( tol < 1 );
        assert( tolLivePointSelection > 0 );
        assert( tolLivePointSelection < 1 );

        // If unit displacement do not use polynomial for selection
        if ( enabled && !livePointSelection && rbf->polynomialTerm )
        {
            WarningIn( "RBFCoarsening::RBFCoarsening" )
                << "Unit displacement is combined with polynomial addition into RBF interpolation. Could cause 'strange' results." << endl;
        }
    }

    RBFCoarsening::RBFCoarsening(
        std::shared_ptr<RBFInterpolation> rbf,
        bool enabled,
        bool livePointSelection,
        bool livePointSelectionSumValues,
        scalar tol,
        scalar tolLivePointSelection,
        int coarseningMinPoints,
        int coarseningMaxPoints,
        bool twoPointSelection,
        bool exportTxt
        )
        :
        rbf( rbf ),
        rbfCoarse( std::shared_ptr<RBFInterpolation> ( new RBFInterpolation( rbf->rbfFunction, rbf->polynomialTerm, rbf->cpu ) ) ),
        enabled( enabled ),
        livePointSelection( livePointSelection ),
        livePointSelectionSumValues( livePointSelectionSumValues ),
        tol( tol ),
        tolLivePointSelection( tolLivePointSelection ),
        coarseningMinPoints( coarseningMinPoints ),
        coarseningMaxPoints( coarseningMaxPoints ),
        twoPointSelection( twoPointSelection ),
        surfaceCorrection( false ),
        ratioRadiusError( 10.0 ),
        exportTxt( exportTxt ),
        selectedPositions(),
        nbStaticFaceCentersRemove( 0 ),
        positions(),
        positionsInterpolation(),
        values(),
        errorInterpolationCoarse(),
        closestBoundaryIndexCorrection(),
        valuesCorrection(),
        nbMovingFaceCenters( 0 ),
        fileExportIndex( 0 )
    {
        assert( rbf );
        assert( coarseningMinPoints <= coarseningMaxPoints );
        assert( coarseningMinPoints > 0 );
        assert( coarseningMaxPoints > 0 );
        assert( tol > 0 );
        assert( tol < 1 );
        assert( tolLivePointSelection > 0 );
        assert( tolLivePointSelection < 1 );

        // If unit displacement do not use polynomial for selection
        if ( enabled && !livePointSelection && rbf->polynomialTerm )
        {
            WarningIn( "RBFCoarsening::RBFCoarsening" )
                << "Unit displacement is combined with polynomial addition into RBF interpolation. Could cause 'strange' results." << endl;
        }
    }

    RBFCoarsening::RBFCoarsening(
        std::shared_ptr<RBFInterpolation> rbf,
        bool enabled,
        bool livePointSelection,
        bool livePointSelectionSumValues,
        scalar tol,
        scalar tolLivePointSelection,
        int coarseningMinPoints,
        int coarseningMaxPoints,
        bool twoPointSelection,
        bool surfaceCorrection,
        scalar ratioRadiusError,
        bool exportTxt
        )
        :
        rbf( rbf ),
        rbfCoarse( std::shared_ptr<RBFInterpolation> ( new RBFInterpolation( rbf->rbfFunction, rbf->polynomialTerm, rbf->cpu ) ) ),
        enabled( enabled ),
        livePointSelection( livePointSelection ),
        livePointSelectionSumValues( livePointSelectionSumValues ),
        tol( tol ),
        tolLivePointSelection( tolLivePointSelection ),
        coarseningMinPoints( coarseningMinPoints ),
        coarseningMaxPoints( coarseningMaxPoints ),
        twoPointSelection( twoPointSelection ),
        surfaceCorrection( surfaceCorrection ),
        ratioRadiusError( ratioRadiusError ),
        exportTxt( exportTxt ),
        selectedPositions(),
        nbStaticFaceCentersRemove( 0 ),
        positions(),
        positionsInterpolation(),
        values(),
        errorInterpolationCoarse(),
        closestBoundaryIndexCorrection(),
        valuesCorrection(),
        nbMovingFaceCenters( 0 ),
        fileExportIndex( 0 )
    {
        assert( rbf );
        assert( coarseningMinPoints <= coarseningMaxPoints );
        assert( coarseningMinPoints > 0 );
        assert( coarseningMaxPoints > 0 );
        assert( tol > 0 );
        assert( tol < 1 );
        assert( tolLivePointSelection > 0 );
        assert( tolLivePointSelection < 1 );

        // If unit displacement do not use polynomial for selection
        if ( enabled && !livePointSelection && rbf->polynomialTerm )
        {
            WarningIn( "RBFCoarsening::RBFCoarsening" )
                << "Unit displacement is combined with polynomial addition into RBF interpolation. Could cause 'strange' results." << endl;
        }
    }

    /* Select a subset of control point with a greedy algorithm.
     * The selection of the points is based on a displacement/motion of
     * a unit displacement at every control point. Based on a user specified tolerance,
     * the control points are selected.
     */
    void RBFCoarsening::compute(
        const matrix & positions,
        const matrix & positionsInterpolation
        )
    {
        this->positions = positions;
        this->positionsInterpolation = positionsInterpolation;
    }

    void RBFCoarsening::greedySelection( const matrix & values )
    {
        assert( positions.cols() == positionsInterpolation.cols() );
        assert( positions.cols() > 0 );
        assert( positions.rows() > 0 );
        assert( positionsInterpolation.rows() > 0 );
        assert( positions.rows() == values.rows() );

        matrix usedPositions = positions;

        if ( enabled )
        {
            // Greedy algorithm

            rbf::vector errorList( positions.rows() );
            selectedPositions.resize( 2 );

            std::unordered_map<int, bool> positionsMap;

            if ( livePointSelection )
            {
                // Select the point with the largest displacment
                int maxDisplacementIndex = -1;
                ( values.rowwise().norm() ).maxCoeff( &maxDisplacementIndex );

                // Add first point
                selectedPositions( 0 ) = maxDisplacementIndex;
            }
            else
            {
                // With unit displacement, first point is point with largest radius from origin and displacement > 0
                vector rad = positions.rowwise().norm();

                int maxRadiusFromOriginIndex = -1;
                double maxRadius = -1;

                for ( int i = 0; i < rad.rows(); i++ )
                {
                    if ( rad( i ) > maxRadius && values.row( i ).norm() > SMALL )
                    {
                        maxRadius = rad( i );
                        maxRadiusFromOriginIndex = i;
                    }
                }

                selectedPositions( 0 ) = maxRadiusFromOriginIndex;
            }

            // Find point with largest distance from first point
            vector rad = ( positions - ( matrix::Constant( positions.rows(), 1, 1.0 ) * positions.row( selectedPositions( 0 ) ) ) ).rowwise().norm();
            int maxRadiusIndex = -1;
            double maxRadius = -1;

            for ( int i = 0; i < rad.rows(); i++ )
            {
                if ( rad( i ) > maxRadius && (rad( i ) < 1.0 - SMALL || rad( i ) > 1.0 + SMALL) )
                {
                    maxRadius = rad( i );
                    maxRadiusIndex = i;
                }
            }

            // Add second point
            selectedPositions( 1 ) = maxRadiusIndex;

            positionsMap[selectedPositions( 0 )] = true;
            positionsMap[selectedPositions( 1 )] = true;

            assert( selectedPositions( 0 ) != selectedPositions( 1 ) );
            assert( positions.rows() >= selectedPositions.rows() );

            rbf::matrix positionsInterpolationCoarse = positions;

            int maxNbPoints = std::min( coarseningMaxPoints, static_cast<int>( positions.rows() ) );
            int minPoints = std::min( coarseningMinPoints, static_cast<int>( positions.rows() ) );
            scalar error = 0;
            scalar errorMax = 0;

            // Create RBF interpolator

            // Run the greedy algorithm
            int counter = selectedPositions.rows();

            while ( true )
            {
                // Build the matrices used for the RBF interpolation
                rbf::matrix positionsCoarse( counter, positions.cols() );
                rbf::matrix valuesCoarse( positionsCoarse.rows(), values.cols() );
                rbf::matrix valuesInterpolationCoarse( positionsInterpolationCoarse.rows(), positionsInterpolationCoarse.cols() );

                for ( int j = 0; j < selectedPositions.rows(); j++ )
                {
                    positionsCoarse.row( j ) = positions.row( selectedPositions( j ) );
                    valuesCoarse.row( j ) = values.row( selectedPositions( j ) );
                }

                // Perform the RBF interpolation.
                std::unique_ptr<RBFInterpolation> rbfCoarse( new RBFInterpolation( rbf->rbfFunction, rbf->polynomialTerm, rbf->cpu ) );
                rbfCoarse->interpolate( positionsCoarse, positionsInterpolationCoarse, valuesCoarse, valuesInterpolationCoarse );

                // Evaluate the error
                for ( int j = 0; j < valuesInterpolationCoarse.rows(); j++ )
                    errorList( j ) = ( valuesInterpolationCoarse.row( j ) - values.row( j ) ).norm();

                // Select the point with the largest error which is not already selected.
                int index = -1;
                scalar largestError = -1;

                for ( int i = 0; i < errorList.rows(); i++ )
                {
                    bool alreadySelected = positionsMap.find( i ) != positionsMap.end();
                    bool largerError = errorList( i ) > largestError;

                    if ( !alreadySelected && largerError )
                    {
                        index = i;
                        largestError = errorList( i );
                    }
                }

                int index2 = -1;
                scalar largestError2 = -1;

                // selected point with largest error in opposite direction (more than 90 degrees differenc in direction)
                if ( twoPointSelection )
                {
                    vector largestErrorVector = valuesInterpolationCoarse.row( index ) - values.row( index );

                    for ( int j = 0; j < errorList.rows(); j++ )
                    {
                        vector errorVector = valuesInterpolationCoarse.row( j ) - values.row( j );

                        if ( largestErrorVector.dot( errorVector ) < -SMALL && largestError2 < errorList( j ) )
                        {
                            index2 = j;
                            largestError2 = errorList( j );
                        }
                    }
                }

                scalar epsilon = std::sqrt( SMALL );
                error = (errorList).norm() / (values.norm() + epsilon);
                errorMax = largestError / ( ( values.rowwise().norm() ).maxCoeff() + epsilon );

                // bool convergence = (error < tol && counter >= minPoints) || counter >= maxNbPoints;
                bool convergence = (error < tol && errorMax < tol && counter >= minPoints) || counter >= maxNbPoints;

                if ( convergence )
                {
                    if ( livePointSelection )
                        errorInterpolationCoarse = valuesInterpolationCoarse - values;

                    break;
                }

                // Break if maximum point are reached
                if ( counter >= maxNbPoints )
                {
                    if ( livePointSelection )
                        errorInterpolationCoarse = valuesInterpolationCoarse - values;

                    break;
                }

                selectedPositions.conservativeResize( selectedPositions.rows() + 1 );
                selectedPositions( selectedPositions.rows() - 1 ) = index;
                counter++;

                positionsMap[index] = true;

                // Add second point if possible
                if ( twoPointSelection && index2 >= 0 && index != index2 )
                {
                    selectedPositions.conservativeResize( selectedPositions.rows() + 1 );
                    selectedPositions( selectedPositions.rows() - 1 ) = index2;
                    counter++;

                    positionsMap[index2] = true;
                }
            }

            Info << "RBF interpolation coarsening: selected " << selectedPositions.rows() << "/" << positions.rows() << " points, 2-norm(error) = "
                 << error << ", max(error) = " << errorMax << ", tol = " << tol << endl;

            rbf::matrix positionsCoarse( selectedPositions.rows(), positions.cols() );

            for ( int i = 0; i < selectedPositions.rows(); i++ )
                positionsCoarse.row( i ) = positions.row( selectedPositions( i ) );

            usedPositions = positionsCoarse;
        }

        rbf->compute( usedPositions, positionsInterpolation );
    }

    void RBFCoarsening::interpolate(
        const matrix & values,
        matrix & valuesInterpolation
        )
    {
        matrix usedValues = values;

        if ( enabled )
        {
            if ( livePointSelection )
            {
                // For RBF mesh interpolation, the values to be interpolated need to be
                // the total displacements. As input, the incremental displacements
                // are given.
                if ( livePointSelectionSumValues )
                {
                    if ( this->values.cols() != values.cols() )
                        this->values = values;
                    else
                        this->values.array() += values.array();
                }
                else
                    this->values = values;

                // Check if re-selection is necessary
                bool reselection = true;

                if ( rbfCoarse->computed )
                {
                    rbf::matrix valuesCoarse( selectedPositions.rows(), values.cols() );
                    rbf::matrix valuesInterpolationCoarse( positions.rows(), valuesInterpolation.cols() );

                    for ( int j = 0; j < selectedPositions.rows(); j++ )
                        valuesCoarse.row( j ) = this->values.row( selectedPositions( j ) );

                    rbfCoarse->interpolate2( valuesCoarse, valuesInterpolationCoarse );

                    scalar epsilon = std::sqrt( SMALL );

                    errorInterpolationCoarse = valuesInterpolationCoarse - this->values;
                    scalar error = (errorInterpolationCoarse).matrix().norm() / (this->values.norm() + epsilon);
                    scalar errorMax = ( errorInterpolationCoarse.rowwise().norm() ).maxCoeff() / ( ( this->values.rowwise().norm() ).maxCoeff() + epsilon );

                    // bool convergence = error < tolLivePointSelection;
                    bool convergence = (error < tolLivePointSelection && errorMax < tolLivePointSelection);

                    if ( convergence )
                        reselection = false;

                    Info << "RBF interpolation coarsening: 2-norm(error) = " << error << ", max(error) = " << errorMax << ", tol = " << tolLivePointSelection << ", reselection = ";

                    if ( reselection )
                        Info << "true";
                    else
                        Info << "false";

                    Info << endl;
                }

                if ( reselection )
                {
                    greedySelection( this->values );

                    rbf->Hhat.conservativeResize( rbf->Hhat.rows(), rbf->Hhat.cols() - nbStaticFaceCentersRemove );
                }
            }
            else
            if ( !rbf->computed )
            {
                // Unit displacement of control points
                matrix unitDisplacement( positions.rows(), positions.cols() );
                unitDisplacement.setZero();

                assert( unitDisplacement.rows() >= nbMovingFaceCenters );

                if ( nbMovingFaceCenters == 0 )
                    unitDisplacement.fill( 1 );
                else
                    for ( int i = 0; i < nbMovingFaceCenters; i++ )
                        for ( int j = 0; j < unitDisplacement.cols(); j++ )
                            unitDisplacement( i, j ) = 1;

                greedySelection( unitDisplacement );

                rbf->Hhat.conservativeResize( rbf->Hhat.rows(), rbf->Hhat.cols() - nbStaticFaceCentersRemove );
            }

            rbf::matrix selectedValues( selectedPositions.rows(), values.cols() );

            for ( int i = 0; i < selectedValues.rows(); i++ )
                selectedValues.row( i ) = values.row( selectedPositions( i ) );

            usedValues = selectedValues;
        }
        else
        {
            if ( !rbf->computed )
            {
                rbf->compute( positions, positionsInterpolation );
                rbf->Hhat.conservativeResize( rbf->Hhat.rows(), rbf->Hhat.cols() - nbStaticFaceCentersRemove );
            }
        }

        usedValues.conservativeResize( usedValues.rows() - nbStaticFaceCentersRemove, usedValues.cols() );
        rbf->interpolate( usedValues, valuesInterpolation );

        // start doing correction of surface is requested
        if ( livePointSelection && surfaceCorrection )
        {
            correctSurface( valuesInterpolation );
        }
    }

    void RBFCoarsening::correctSurface( matrix & valuesInterpolation )
    {
        if ( valuesCorrection.rows() == 0 )
        {
            valuesCorrection.conservativeResize( valuesInterpolation.rows(), valuesInterpolation.cols() );
            valuesCorrection.setZero();
        }

        scalar R = ratioRadiusError * ( errorInterpolationCoarse.rowwise().norm() ).maxCoeff();

        // Find nearest boundary point for each internal point. Do this only the first time
        vector closestBoundaryRadius( positionsInterpolation.rows() );

        if ( closestBoundaryIndexCorrection.rows() == 0 )
        {
            closestBoundaryIndexCorrection.conservativeResize( positionsInterpolation.rows() );

            for ( int i = 0; i < positionsInterpolation.rows(); i++ )
            {
                scalar smallestRadius = GREAT;
                int boundaryIndex = -1;

                for ( int j = 0; j < positions.rows(); j++ )
                {
                    scalar radius = ( positions.row( j ) - positionsInterpolation.row( i ) ).norm();

                    if ( radius < smallestRadius )
                    {
                        boundaryIndex = j;
                        smallestRadius = radius;
                    }
                }

                closestBoundaryIndexCorrection( i ) = boundaryIndex;
                closestBoundaryRadius( i ) = smallestRadius;
            }
        }
        else
        {
            for ( int i = 0; i < positionsInterpolation.rows(); i++ )
            {
                closestBoundaryRadius( i ) = ( positions.row( closestBoundaryIndexCorrection( i ) ) - positionsInterpolation.row( i ) ).norm();
            }
        }

        // Start doing the correction
        std::shared_ptr<rbf::RBFFunctionInterface> rbfFunction = std::shared_ptr<rbf::RBFFunctionInterface> ( new rbf::WendlandC2Function( R ) );

        for ( int i = 0; i < positionsInterpolation.rows(); i++ )
        {
            matrix fEval = -( rbfFunction->evaluate( closestBoundaryRadius( i ) ) ) * errorInterpolationCoarse.row( closestBoundaryIndexCorrection( i ) );
            valuesInterpolation.row( i ) += ( fEval - valuesCorrection.row( i ) );
            valuesCorrection.row( i ) = fEval;
        }
    }

    void RBFCoarsening::setNbMovingAndStaticFaceCenters(
        int nbMovingFaceCenters,
        int nbStaticFaceCenters
        )
    {
        nbStaticFaceCentersRemove = nbStaticFaceCenters;
        this->nbMovingFaceCenters = nbMovingFaceCenters;

        if ( enabled )
        {
            // Determine the number of selected static face centers
            nbStaticFaceCentersRemove = 0;

            for ( int i = 0; i < selectedPositions.rows(); i++ )
                if ( selectedPositions( i ) >= nbMovingFaceCenters )
                    nbStaticFaceCentersRemove++;
        }
    }
}
