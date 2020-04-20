/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License version 2 as
*   published by the Free Software Foundation;
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*/

#ifndef SRC_NR_MODEL_BEAMFORMING_VECTOR_H_
#define SRC_NR_MODEL_BEAMFORMING_VECTOR_H_

#include "beam-id.h"
#include <ns3/three-gpp-antenna-array-model.h>

namespace ns3{

typedef std::vector<std::complex<double>> complexVector_t; //!< type definition for complex vectors

/**
 * \ingroup utils
 * \brief Physical representation of a beam.
 *
 * Contains the vector of the antenna weight, as well as the beam id. These
 * values are stored as std::pair, and we provide utilities functions to
 * extract them.
 *
 * \see GetVector
 * \see GetBeamId
 */
typedef std::pair<complexVector_t, BeamId>  BeamformingVector;


/**
 * \brief Create a quasi omni beamforming vector
 * \ingroup utils
 * \param antennaRows Number of rows in antenna array
 * \param antennaColumns Number of columns in the antenna array
 * \return the beamforming vector
 */
complexVector_t CreateQuasiOmniBfv (uint32_t antennaRows, uint32_t antennaColumns);

/**
 * \brief Creates a beamforming vector for a given sector and elevation
 * \ingroup utils
 * \param antenna Antenna array for which will be created the beamforming vector
 * \param sector sector to be used
 * \param elevation elevation to be used
 * \return the beamforming vector
 */
complexVector_t CreateDirectionalBfv (const Ptr<const ThreeGppAntennaArrayModel>& antenna, uint16_t sector, double elevation);

}

#endif /* SRC_NR_MODEL_BEAMFORMING_VECTOR_H_ */
