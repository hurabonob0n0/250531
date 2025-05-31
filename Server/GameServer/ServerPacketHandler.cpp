#include "pch.h"
#include "SGlobal.h"
#include "ServerPacketHandler.h"
#include "BufferReader.h"
#include "BufferWriter.h"
#include "ClientSession.h"
#include "Player.h"
#include "Room.h"

void ServerPacketHandler::HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br.Peek(&header);

	switch (header.id)
	{
	case C_LOGIN:
		Handle_C_LOGIN(session,buffer, len);
		break;
	case C_KEYINPUT:
		Handle_C_KEYINPUT(session, buffer, len);
		break;

	case C_MOVEMENT:
		Handle_C_MOVEMENT(session, buffer, len);
		break;

	default:
		break;
	}
}

void ServerPacketHandler::Handle_C_LOGIN(PacketSessionRef& session,BYTE* buffer, int32 len)
{

	ClientSessionRef C_Session = static_pointer_cast<ClientSession>(session);
	
	static Atomic<uint64> ClientID = 0;
	uint64 newClientID = ClientID.fetch_add(1);

	PlayerRef playerRef = MakeShared<Player>();
	playerRef->playerID = newClientID;
	playerRef->OwenerSession = C_Session;
	C_Session->_players.push_back(playerRef);

	GRoom.Accept_Player(playerRef);
	cout << "Player" << playerRef->playerID << "EnterRoom" << endl;

	auto sendbuffer = Make_S_SUCCES_LOGIN((uint16)newClientID);
	session->Send(sendbuffer);
		

	//로그인


}


// -> 또 큐 => 룸마다 관리 같은데에 데이터를 쌓아둔다.
//서버 프레임돌때 첫 프레임에 => 큐에 모든 데이터 처리후 객체들 업데이트 -> 브로드캐스트
//키인풋->

void ServerPacketHandler::Handle_C_KEYINPUT(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	ClientSessionRef C_Session = static_pointer_cast<ClientSession>(session);
	
	int64 id;
	if (!C_Session->_players.empty())
		id = C_Session->_players[0]->playerID;

	BufferReader br(buffer, len);
	PacketHeader header;
	br >> header;

	uint8 key;

	br >> key;

	cout << "Player(" << id << ") Moved" << endl;
	
	
	//GRoom.MovePlayer(id);


	// 속도 설정.
	// 교수님이 원하시는게 탱크 궤도.. 댐핑 -> 서버에서 구현한다 ?
	//
	// 따로 쓰레드 하나 만들어서 -> firstDown

}

void ServerPacketHandler::Handle_C_MOVEMENT(PacketSessionRef& session, BYTE* buffer, int32 len)
{

	ClientSessionRef C_Session = static_pointer_cast<ClientSession>(session);


	BufferReader br(buffer, len);
	PacketHeader header;
	br >> header;
	
	float x1;
	float y1;
	float z1;
	br >> x1 >> y1 >> z1;

	if (!C_Session->_players.empty()) {
		uint64 ID = C_Session->_players[0]->playerID;
		GRoom.SetPlayerPos(ID,x1,y1,z1);
		GRoom.isStart = true;
	}

	//if (!C_Session->_players.empty()) {
	//	uint64 ID = C_Session->_players[0]->playerID;
	//	Vec3_Data Temp;
	//	if (ID == 0) {
	//		Temp = GRoom.GetPlayerPos(1);

	//	}
	//	else {
	//		Temp = GRoom.GetPlayerPos(0);

	//	}


	//	auto sendbuffer = Make_S_PLAYER_MOVED(Temp);
	//	session->Send(sendbuffer);
	//}

}

SendBufferRef ServerPacketHandler::Make_S_TEST(uint64 id, uint32 hp, uint16 attack)
{
	SendBufferRef sendBuffer = GSendBufferManager->Open(4096);

	BufferWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());

	PacketHeader* header = bw.Reserve<PacketHeader>();
	
	bw << id << hp << attack;



	header->size = bw.WriteSize();
	header->id = S_TEST; // 1 : Test Msg

	sendBuffer->Close(bw.WriteSize());

	return sendBuffer;
}

SendBufferRef ServerPacketHandler::Make_S_ROOMCREATED(uint16 id)
{
	SendBufferRef sendBuffer = GSendBufferManager->Open(4096);

	BufferWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());

	PacketHeader* header = bw.Reserve<PacketHeader>();

	bw << id;

	header->size = bw.WriteSize();
	header->id = S_ROOMCREATED;

	cout << "Room Created! -> send for Brodcast" << endl;

	return sendBuffer;
}

SendBufferRef ServerPacketHandler::Make_S_SUCCES_LOGIN(uint16 id)
{

	SendBufferRef sendBuffer = GSendBufferManager->Open(4096);

	BufferWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());

	PacketHeader* header = bw.Reserve<PacketHeader>();

	bw << id;						// 먼저 데이터를 쓴다
	header->size = bw.WriteSize();	// 그다음 정확한 전체 크기를 설정한다
	header->id = S_SUCCES_LOGIN;



	sendBuffer->Close(bw.WriteSize());

	return sendBuffer;
}

SendBufferRef ServerPacketHandler::Make_S_PLAYER_MOVED(Vec3_Data Position)
{
	SendBufferRef sendBuffer = GSendBufferManager->Open(4096);
	BufferWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());

	PacketHeader* header = bw.Reserve<PacketHeader>();

	// float 그대로 전송

	
	Vec3_Data& pos = Position;
	bw << pos.X << pos.Y << pos.Z;
	

	header->size = bw.WriteSize();
	header->id = S_PLAYER_MOVE;

	sendBuffer->Close(bw.WriteSize());
	return sendBuffer;
}
