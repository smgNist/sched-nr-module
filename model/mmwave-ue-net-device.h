/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#ifndef SRC_MMWAVE_MODEL_MMWAVE_UE_NET_DEVICE_H_
#define SRC_MMWAVE_MODEL_MMWAVE_UE_NET_DEVICE_H_

#include "mmwave-net-device.h"

namespace ns3 {

class Packet;
class PacketBurst;
class Node;
class MmWaveUePhy;
class MmWaveUeMac;
class LteUeComponentCarrierManager;
class EpcUeNas;
class LteUeRrc;
class MmWaveEnbNetDevice;
class ComponentCarrierMmWaveUe;

class MmWaveUeNetDevice : public MmWaveNetDevice
{

public:
  static TypeId GetTypeId (void);

  MmWaveUeNetDevice (void);

  virtual ~MmWaveUeNetDevice (void);

  uint32_t GetCsgId () const;

  void SetCsgId (uint32_t csgId);


  virtual Ptr<MmWavePhy> GetPhy (uint8_t index) const;

  /**
   * Returns the PHY instance that is configured to operate on the provided
   * central carrier frequency
   * @param centerFrequency The central carrier frequency in Hz
   * @return A pointer to the PHY instance it it exist, otherwise a nullptr
   */
  virtual Ptr<MmWavePhy> GetPhyOnCenterFreq (double centerFrequency) const;

  Ptr<LteUeComponentCarrierManager> GetComponentCarrierManager (void) const;

  uint64_t GetImsi () const;

  uint16_t GetEarfcn () const;

  Ptr<EpcUeNas> GetNas (void) const;

  Ptr<LteUeRrc> GetRrc () const;

  void SetEarfcn (uint16_t earfcn);

  void SetTargetEnb (Ptr<MmWaveEnbNetDevice> enb);

  Ptr<MmWaveEnbNetDevice> GetTargetEnb (void);

  /**
   * \brief Set the ComponentCarrier Map for the UE
   * \param ccm the map of ComponentCarrierUe
   */
  void SetCcMap (std::map< uint8_t, Ptr<ComponentCarrierMmWaveUe> > ccm);

  /**
   * \brief Get the ComponentCarrier Map for the UE
   * \returns the map of ComponentCarrierUe
   */
  std::map< uint8_t, Ptr<ComponentCarrierMmWaveUe> >  GetCcMap (void);

  /**
   * \brief Get the size of the component carriers map
   * \return the number of cc that we have
   */
  uint32_t GetCcMapSize () const;

protected:
  // inherited from Object
  virtual void DoInitialize (void);
  virtual void DoDispose ();

  void UpdateConfig (void);

  virtual bool DoSend (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);

private:
  Ptr<MmWaveEnbNetDevice> m_targetEnb;
  Ptr<LteUeRrc> m_rrc;
  Ptr<EpcUeNas> m_nas;
  uint64_t m_imsi;
  uint16_t m_earfcn;
  uint32_t m_csgId;
  bool m_isConstructed;

  std::map < uint8_t, Ptr<ComponentCarrierMmWaveUe> > m_ccMap; ///< component carrier map
  Ptr<LteUeComponentCarrierManager> m_componentCarrierManager; ///< the component carrier manager

};

}
#endif /* SRC_MMWAVE_MODEL_MMWAVE_UE_NET_DEVICE_H_ */
