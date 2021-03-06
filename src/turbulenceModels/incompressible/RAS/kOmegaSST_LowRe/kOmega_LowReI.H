/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | foam-extend: Open Source CFD
   \\    /   O peration     |
    \\  /    A nd           | For copyright notice see file Copyright
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of foam-extend.

    foam-extend is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    foam-extend is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with foam-extend.  If not, see <http://www.gnu.org/licenses/>.

Global
    kOmegaWallFunctions

Description
    Calculate wall generation and frequency omega from wall-functions.

\*---------------------------------------------------------------------------*/

{
    labelList cellBoundaryFaceCount(omega_.size(), 0);

    scalar Cmu25 = pow(Cmu_.value(), 0.25);

    const fvPatchList& patches = mesh_.boundary();

    //- Initialise the near-wall omega and G fields to zero
    forAll(patches, patchi)
    {
        const fvPatch& curPatch = patches[patchi];

        if (isType<wallFvPatch>(curPatch))
        {
            forAll(curPatch, facei)
            {
                label faceCelli = curPatch.faceCells()[facei];

                omega_[faceCelli] = 0.0;
                G[faceCelli] = 0.0;
            }
        }
    }

    //- Accumulate the wall face contributions to omega and G
    //  Increment cellBoundaryFaceCount for each face for averaging
    forAll(patches, patchi)
    {
        const fvPatch& curPatch = patches[patchi];

        if (isType<wallFvPatch>(curPatch))
        {
#           include "checkkOmega_LowRePatchFieldTypes.H"

            const scalarField& nuw = nu().boundaryField()[patchi];
            const scalarField& nutw = nut_.boundaryField()[patchi];

            scalarField magFaceGradU =
                mag(U_.boundaryField()[patchi].snGrad());

            forAll(curPatch, facei)
            {
                label faceCelli = curPatch.faceCells()[facei];

                // For corner cells (with two boundary or more faces),
                // omega and G in the near-wall cell are calculated
                // as an average

                cellBoundaryFaceCount[faceCelli]++;

                omega_[faceCelli] += scalar(6)*nuw[facei]
                    /(beta1_.value()*sqr(y_[faceCelli]));

                G[faceCelli] +=
                    (nutw[facei] + nuw[facei])*magFaceGradU[facei]
                    *Cmu25*sqrt(k_[faceCelli])/(kappa_.value()*y_[faceCelli]);
            }
        }
    }


    // Perform the averaging

    forAll(patches, patchi)
    {
        const fvPatch& curPatch = patches[patchi];

        if (isType<wallFvPatch>(curPatch))
        {
            forAll(curPatch, facei)
            {
                label faceCelli = curPatch.faceCells()[facei];

                omega_[faceCelli] /= cellBoundaryFaceCount[faceCelli];
                G[faceCelli] /= cellBoundaryFaceCount[faceCelli];
            }
        }
    }
}


// ************************************************************************* //
