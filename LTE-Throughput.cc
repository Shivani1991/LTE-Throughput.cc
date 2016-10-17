/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Manuel Requena <manuel.requena@cttc.es>
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/config-store.h"
#include <ns3/buildings-helper.h>


using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("LTE-Throughput");



uint64_t lastTotalRx =0;

void
CalculateThroughput (uint64_t cur)
{
  Time now = Simulator::Now ();                                                            /* Return the simulator's virtual time. */
   
  // double cur = (rlcStats->GetDlRxPackets  (1, 3) - lastTotalRx) * (double) 8/1e5;       /* Convert Application RX Packets to MBits. */
  std::cout << now.GetSeconds () << "s: \t" << cur << " MB" << std::endl;
  // std::cout << rlcStats->GetDlRxPackets  (1, 3)<<std::endl;
  // lastTotalRx = cur;
  Simulator::Schedule (MilliSeconds (100), &CalculateThroughput,cur);
}





int main (int argc, char *argv[])
{	
  // LogComponentEnable ("RadioBearerStatsCalculator", LOG_LEVEL_ALL);
  
  CommandLine cmd;
  cmd.Parse (argc, argv);

	
  // to save a template default attribute file run it like this:
  // ./waf --command-template="%s --ns3::ConfigStore::Filename=input-defaults.txt --ns3::ConfigStore::Mode=Save --  
  // ns3::ConfigStore::FileFormat=RawText" --run src/lte/examples/lena-first-sim
  //
  // to load a previously created default attribute file
  // ./waf --command-template="%s --ns3::ConfigStore::Filename=input-defaults.txt --ns3::ConfigStore::Mode=Load --
  // ns3::ConfigStore::FileFormat=RawText" --run src/lte/examples/lena-first-sim


  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  // Parse again so you can override default values from the command line
  cmd.Parse (argc, argv);

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  // Uncomment to enable logging
  // lteHelper->EnableLogComponents ();


  // Create Nodes: eNodeB and UE
  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create (1);
  ueNodes.Create (1);


  // Install Mobility Model
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (enbNodes);
  BuildingsHelper::Install (enbNodes);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (ueNodes);
  BuildingsHelper::Install (ueNodes);


  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  // Default scheduler is PF, uncomment to use RR
  // lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");


  enbDevs = lteHelper->InstallEnbDevice (enbNodes);
  ueDevs = lteHelper->InstallUeDevice (ueNodes);

  // Attach a UE to a eNB
  lteHelper->Attach (ueDevs, enbDevs.Get (0));


  // Activate a data radio bearer
  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearer (q);
  lteHelper->ActivateDataRadioBearer (ueDevs, bearer);

  
  lteHelper->EnableTraces ();

  Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats ();

  // std::cout<<"\n**** Starting Simulation *****"<<rlcStats->GetStartTime ()<< std::endl;
  std::cout<<"\n\t\t**** Starting Simulation *****\n"<< std::endl;
  std::cout<<"Algo. \tTime(s) \tTotal Packet \t PacketSize(B) \t Throughput(Mbps)" <<std::endl; 

  Simulator::Stop (Seconds (1));
  Simulator::Run ();
  
  // uint64_t cur=rlcStats->GetDlRxData(1,3);
  // std::cout<<cur<< std::endl;
  // Simulator::Schedule (Seconds (1.1), &CalculateThroughput,cur);
  // std::cout<<rlcStats->GetDlRxData(1,3)<< std::endl;

  
  
  Time now = Simulator::Now (); 
  std::cout <<"Pf \t"<< now.GetSeconds () << " \t\t" << rlcStats->GetDlTxPackets(1,3) << "\t\t "<< rlcStats->GetDlTxData(1,3)<< "\t\t " << 
  (rlcStats->GetDlTxPackets(1,3)*rlcStats->GetDlTxData(1,3))/(1000000 * now.GetSeconds ())<<std::endl;
  // std::cout<<rlcStats->GetDlRxData(1,3)<<std::endl;
  // GtkConfigStore config;
  // config.ConfigureAttributes ();



  // ***RR**
  Simulator::Stop (Seconds (1));
  Simulator::Run ();

  lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");
  std::cout <<"Rr \t"<< now.GetSeconds () << " \t\t" << rlcStats->GetDlTxPackets(1,3) << "\t\t "<< rlcStats->GetDlTxData(1,3)<< "\t\t " <<  
  (rlcStats->GetDlTxPackets(1,3)*rlcStats->GetDlTxData(1,3))/(1000000 * now.GetSeconds ())<<std::endl;



  // ***FdMt**
  Simulator::Stop (Seconds (1));
  Simulator::Run ();

  lteHelper->SetSchedulerType ("ns3::FdMtFfMacScheduler");
  std::cout <<"FdMt \t"<< now.GetSeconds () << " \t\t" << rlcStats->GetDlTxPackets(1,3) << "\t\t "<< rlcStats->GetDlTxData(1,3)<< "\t\t " <<    
  (rlcStats->GetDlTxPackets(1,3)*rlcStats->GetDlTxData(1,3))/(1000000 * now.GetSeconds ())<<std::endl;



  // ***TdMt**
  Simulator::Stop (Seconds (1));
  Simulator::Run ();

  lteHelper->SetSchedulerType ("ns3::TdMtFfMacScheduler");
  std::cout <<"TdMt \t"<< now.GetSeconds () << " \t\t" << rlcStats->GetDlTxPackets(1,3) << "\t\t "<< rlcStats->GetDlTxData(1,3)<< "\t\t " <<   
  (rlcStats->GetDlTxPackets(1,3)*rlcStats->GetDlTxData(1,3))/(1000000 * now.GetSeconds ())<<std::endl;



  // ***Tta**
  Simulator::Stop (Seconds (1));
  Simulator::Run ();

  lteHelper->SetSchedulerType ("ns3::TtaFfMacScheduler"); 
  std::cout <<"Tta \t"<< now.GetSeconds () << " \t\t" << rlcStats->GetDlTxPackets(1,3) << "\t\t "<< rlcStats->GetDlTxData(1,3)<< "\t\t " <<   
  (rlcStats->GetDlTxPackets(1,3)*rlcStats->GetDlTxData(1,3))/(1000000 * now.GetSeconds ())<<std::endl;

 
  // ***FdBet**
  Simulator::Stop (Seconds (1));
  Simulator::Run ();

  lteHelper->SetSchedulerType ("ns3::FdBetFfMacScheduler");
  std::cout <<"FdBet \t"<< now.GetSeconds () << " \t\t" << rlcStats->GetDlTxPackets(1,3) << "\t\t "<< rlcStats->GetDlTxData(1,3)<< "\t\t " <<   
  (rlcStats->GetDlTxPackets(1,3)*rlcStats->GetDlTxData(1,3))/(1000000 * now.GetSeconds ())<<std::endl; 

  

  // ***TdBet**
  Simulator::Stop (Seconds (1));
  Simulator::Run ();

  lteHelper->SetSchedulerType ("ns3::TdBetFfMacScheduler");
  std::cout <<"TdBet \t"<< now.GetSeconds () << " \t\t" << rlcStats->GetDlTxPackets(1,3) << "\t\t "<< rlcStats->GetDlTxData(1,3)<< "\t\t " <<   
  (rlcStats->GetDlTxPackets(1,3)*rlcStats->GetDlTxData(1,3))/(1000000 * now.GetSeconds ())<<std::endl; 



  // ***FdTbfq**
  Simulator::Stop (Seconds (1));
  Simulator::Run ();

  lteHelper->SetSchedulerType ("ns3::FdTbfqFfMacScheduler");
  std::cout <<"FdTbfq \t"<< now.GetSeconds () << " \t\t" << rlcStats->GetDlTxPackets(1,3) << "\t\t "<< rlcStats->GetDlTxData(1,3)<< "\t\t " <<   
  (rlcStats->GetDlTxPackets(1,3)*rlcStats->GetDlTxData(1,3))/(1000000 * now.GetSeconds ())<<std::endl; 
  


  
  // ***TdTbfq**
  Simulator::Stop (Seconds (1));
  Simulator::Run ();

  lteHelper->SetSchedulerType ("ns3::TdTbfqFfMacScheduler");
  std::cout <<"TdTbfq \t"<< now.GetSeconds () << " \t\t" << rlcStats->GetDlTxPackets(1,3) << "\t\t "<< rlcStats->GetDlTxData(1,3)<< "\t\t " <<   
  (rlcStats->GetDlTxPackets(1,3)*rlcStats->GetDlTxData(1,3))/(1000000 * now.GetSeconds ())<<std::endl;





  // ***Pss**
  Simulator::Stop (Seconds (1));
  Simulator::Run ();

  lteHelper->SetSchedulerType ("ns3::PssFfMacScheduler"); 
  std::cout <<"Pss \t"<< now.GetSeconds () << " \t\t" << rlcStats->GetDlTxPackets(1,3) << "\t\t "<< rlcStats->GetDlTxData(1,3)<< "\t\t " <<   
  (rlcStats->GetDlTxPackets(1,3)*rlcStats->GetDlTxData(1,3))/(1000000 * now.GetSeconds ())<<"\n"<<std::endl; 


  Simulator::Destroy ();
 // std::cout<<rlcStats->GetEpoch ()<<std::endl; 

 return 0;
}
