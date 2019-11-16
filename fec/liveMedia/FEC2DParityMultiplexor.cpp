#include "include/FEC2DParityMultiplexor.hh"
#include <iostream>

FEC2DParityMultiplexor* FEC2DParityMultiplexor::createNew(UsageEnvironment& env, u_int8_t row, u_int8_t column, long long repairWindow) {
    return new FEC2DParityMultiplexor(env, row, column, repairWindow);
}

FEC2DParityMultiplexor::FEC2DParityMultiplexor(UsageEnvironment& env, u_int8_t row, u_int8_t column, long long repairWindow) : FECMultiplexor(env){
    first = True;
    ssrcWasSet = False;
    fRepairWindow = repairWindow;
    fRow = row;
    fColumn = column;
    hostSSRC = 0;

    nextTask() = envir().taskScheduler().scheduleDelayedTask(20000, (TaskFunc*)sendNext, this);
}

FEC2DParityMultiplexor::~FEC2DParityMultiplexor() {

}

void FEC2DParityMultiplexor::pushFECRTPPacket(unsigned char* buffer, unsigned bufferSize) {
	RTPPacket* rtpPacket = RTPPacket::createNew(buffer, bufferSize);
	setHostSSRCIfNotSet(rtpPacket);

	if (first) setBaseIfNotSet(rtpPacket);
    else insertPacket(rtpPacket);
}

void FEC2DParityMultiplexor::sendNext(void* firstArg) {
	FEC2DParityMultiplexor* fec2DParityMultiplexor = (FEC2DParityMultiplexor*)firstArg;
	fec2DParityMultiplexor->repairPackets();
}

void FEC2DParityMultiplexor::repairPackets() {
    Boolean packetsAreAvailable = False;

    int clustersToErase = 0;

	for (int i = 0; i < superBuffer.size(); i++) {
		FECCluster* cluster = superBuffer.at(i);

        if (!cluster->allRTPPacketsArePresent() && !cluster->hasExpired(fRepairWindow)) break;

        if (!cluster->allRTPPacketsArePresent() && cluster->hasExpired(fRepairWindow)) {
            //std::cout << "REPAIR!!\nREPAIR!!!\n";
            FECDecoder::repairCluster(cluster->rtpPackets(), fRow, fColumn, hostSSRC);   //what if ssrc is not set?
        }

        flushCluster(cluster->rtpPackets());
        clustersToErase++;
        packetsAreAvailable = True;
	}
    if (clustersToErase > 0) {
        superBuffer.erase( superBuffer.begin(), (superBuffer.begin() + clustersToErase));
    }

	if (packetsAreAvailable) envir().taskScheduler().triggerEvent(fEventTriggerId, this);
	nextTask() = envir().taskScheduler().scheduleDelayedTask(20000, (TaskFunc*)sendNext, this);
}

void FEC2DParityMultiplexor::setHostSSRCIfNotSet(RTPPacket* rtpPacket) {
    if (ssrcWasSet) return;
    int payload = EXTRACT_BIT_RANGE(0, 7, rtpPacket->content()[1]);
    if (payload == 115 || payload == 116) return;
    hostSSRC = FECDecoder::extractSSRC(rtpPacket);
    ssrcWasSet = True;
}

void FEC2DParityMultiplexor::setBaseIfNotSet(RTPPacket* rtpPacket) {
    emergencyBuffer.push_back(rtpPacket);
    int didFind = False;
    u_int16_t newBase = 0;
    findBase(&didFind, &newBase);
    if (didFind == 0) return;
    //std::cout << newBase << "\n";
    first = False;
    currentSequenceNumber = newBase;
    handleEmergencyBuffer(newBase);
}

void FEC2DParityMultiplexor::insertPacket(RTPPacket* rtpPacket) {
    int payload = EXTRACT_BIT_RANGE(0, 7, rtpPacket->content()[1]);

	u_int16_t seq = (payload == 115 || payload == 116) ? FECDecoder::extractFECBase(rtpPacket) : FECDecoder::extractRTPSeq(rtpPacket);
    if (payload == 96) {
        //std::cout << seq << "\n";
    }
    //std::cout << payload << ": " << seq << "\n";

	int sourcePacketCount = fRow * fColumn;

  u_int16_t newSeq = currentSequenceNumber + sourcePacketCount;

  //Find if special case
  if (newSeq < currentSequenceNumber) {
    /*Special case*/
    if (seq >= currentSequenceNumber || seq < newSeq) {
      //Find cluster
      FECCluster* fecCluster = findCluster(seq);
      if (fecCluster != NULL) fecCluster->insertPacket(rtpPacket);
      else {
          if (payload == 115 || payload == 116) {
              //std::cout << "currentseq: " << currentSequenceNumber << " didnt find cluster for repair seq " << seq << "\n";
          }
          else {
              //std::cout << "currentseq: " << currentSequenceNumber << " didnt find cluster for source seq " << seq << "\n";
          }
          //printSuperBuffer();

      }
    }
    else {
      //Make cluster

      u_int16_t diff = seq - currentSequenceNumber;
      if (diff > 30000) {
          //Arbitrary number. This is a failsafe if a packet arrive out of in the 65535 rollover. Then the packet will be tossed.
      }
      else {
          updateCurrentSequenceNumber(seq, sourcePacketCount);
          FECCluster* fecCluster = FECCluster::createNew(currentSequenceNumber, fRow, fColumn);
          fecCluster->insertPacket(rtpPacket);
          if (fecCluster->hasOnlyNullPackets()) {
              std::cout << "HAS ONLY NULL PACKETS, SEQ: " << seq << " BASE: " << currentSequenceNumber << "\n";
          }

          superBuffer.push_back(fecCluster);
          //std::cout << "making cluster :" << seq << " time: " << fecCluster->timestamp() << "\n";
      }


    }
  }
  else {
    /*Regular case*/
    if (seq >= newSeq) {
      //Make new

      u_int16_t diff = seq - currentSequenceNumber;
      if (diff > 30000) {
          //Arbitrary number. This is a failsafe if a packet arrive out of in the 65535 rollover. Then the packet will be tossed.
      }
      else {
          updateCurrentSequenceNumber(seq, sourcePacketCount);
          FECCluster* fecCluster = FECCluster::createNew(currentSequenceNumber, fRow, fColumn);
          fecCluster->insertPacket(rtpPacket);
          if (fecCluster->hasOnlyNullPackets()) {
              std::cout << "HAS ONLY NULL PACKETS, SEQ: " << seq << " BASE: " << currentSequenceNumber << "\n";
          }

          superBuffer.push_back(fecCluster);
          //std::cout << "making cluster :" << seq << " time: " << fecCluster->timestamp() << "\n";
      }


    }
    else {
      //find
      FECCluster* fecCluster = findCluster(seq);
      if (fecCluster != NULL) {
          fecCluster->insertPacket(rtpPacket);
      }
      else {
          if (payload == 115 || payload == 116) {
              //std::cout << "currentseq: " << currentSequenceNumber << " didnt find cluster for repair seq " << seq << "\n";
          }
          else {
              //std::cout << "currentseq: " << currentSequenceNumber << " didnt find cluster for source seq " << seq << "\n";
          }
          //printSuperBuffer();
      }
    }
  }
}

/*TODO:
	Fiks mod 65536, tror den er OK
	Memleaks
	Håndter flere clustere hvis base er laaangt fram i tid
*/
void FEC2DParityMultiplexor::handleEmergencyBuffer(u_int16_t base) {
	FECCluster* fecCluster = FECCluster::createNew(base, fRow, fColumn);
	Boolean thereWasReadyRTPPackets = False;

	for (int i = 0; i < emergencyBuffer.size(); i++) {
		RTPPacket* current = emergencyBuffer.at(i);

		int payload = EXTRACT_BIT_RANGE(0, 7, current->content()[1]);
        u_int16_t seq = (payload == 115 || payload == 116) ? FECDecoder::extractFECBase(current) : FECDecoder::extractRTPSeq(current);

		if (fecCluster->seqNumInCluster(seq)) fecCluster->insertPacket(current);
		else {
			if (payload != 115 && payload != 116) {	//hvis nyere burde legges i cluster? Logisk umulig?
				fRTPPackets.push(current);
				thereWasReadyRTPPackets = True;
			}
		}
	}
	superBuffer.push_back(fecCluster);

	if (thereWasReadyRTPPackets)
        envir().taskScheduler().triggerEvent(fEventTriggerId, this);
}


/*Utility methods*/
void FEC2DParityMultiplexor::updateCurrentSequenceNumber(u_int16_t newSeqnum, unsigned sourcePacketCount) {
    u_int16_t diff = newSeqnum - currentSequenceNumber;
    u_int16_t clustersBetween = diff / sourcePacketCount;
    currentSequenceNumber = currentSequenceNumber + clustersBetween * sourcePacketCount;
    //std::cout << "new cluster with base: " << currentSequenceNumber << "\n";


}

//mod 65536?
void FEC2DParityMultiplexor::findBase(int* didFind, u_int16_t* newBase) {
	for (int i = 0; i < emergencyBuffer.size(); i++) {
		RTPPacket* curr = emergencyBuffer.at(i);
		int payload = EXTRACT_BIT_RANGE(0, 7, curr->content()[1]);
		if (payload == 115 || payload == 116) {
			u_int16_t currBase = (((u_int16_t)curr->content()[20]) << 8) | curr->content()[21];

			for (int j = 0; j < emergencyBuffer.size(); j++) {
				RTPPacket* lol = emergencyBuffer.at(j);
				int payload2 = EXTRACT_BIT_RANGE(0, 7, lol->content()[1]);
				if (payload2 == (payload == 115 ? 116 : 115)) {
					u_int16_t lolBase = (((u_int16_t)lol->content()[20]) << 8) | lol->content()[21];
					if (currBase == lolBase) {
						*didFind = 1;
						*newBase = lolBase;
						return;
					}
				}
			}
		}
	}
}

FECCluster* FEC2DParityMultiplexor::findCluster(u_int16_t seqNum) {
	for (int i = 0; i < superBuffer.size(); i++) {
		if (superBuffer.at(i)->seqNumInCluster(seqNum)) {
			return superBuffer.at(i);
		}
	}
	return NULL;
}

void FEC2DParityMultiplexor::flushCluster(RTPPacket** cluster) {
    //std::cout << "flusing:\n";
    //FECDecoder::printCluster(cluster, fRow, fColumn);

	int size = fRow * fColumn;
	for (int i = 0; i < size; i++) {
		int index = i + i / fColumn;
		RTPPacket* rtpPacket = cluster[index];
		if (rtpPacket != NULL) {
            //u_int16_t seq = FECDecoder::extractRTPSeq(rtpPacket);
            //std::cout << seq << "\n";
            fRTPPackets.push(rtpPacket);
        }
	}
    //std::cout << "end flusing:\n";
}

void FEC2DParityMultiplexor::printSuperBuffer() {
    if (superBuffer.empty()) {
        std::cout << "superbuffer is empty!\n";
        return;
    }
    std::cout << "Start of superbuffer:\n";
    for (int i = 0; i < superBuffer.size(); i++) {
        FECCluster* cluster = superBuffer.at(i);
        FECDecoder::printCluster(cluster->rtpPackets(), fRow, fColumn);
    }
    std::cout << "End of superbuffer:\n";

}
