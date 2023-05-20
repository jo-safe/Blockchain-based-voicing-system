
// RESPONSES
#pragma once

//static ifstream in1;
//static ofstream out1;

//UnicodeString r_get_peers(KRPCmsg, int, TIdUDPClient*);
//UnicodeString r_announce_peer(KRPCmsg, int, TIdUDPClient*);
//UnicodeString r_ask_ip(KRPCmsg, int, TIdUDPClient*);
//-----------------------------------------------------------------------------
/* ��� ����� �� ��������� ���� get_peers
* � ������ ���� ���� �����, �� ����� ��� ��� ����� � ���� � �� ��������������
* ���������� ��� �������, ����������� ��� �������������� � ����.
* � ������ ���� ���� ���������� get_peers, �� �������� ������ � ���������
* ������ ���� �� ������, ���������� � ���������. �� ��������� � ���� ��������,
* ��������� �����, �, ��� �������������, ������� ��������� ������ ����������.
* ������ ������ ������� � ���, ��� ���� �������� �� ������� �������� �� ��,
* ��������� �� ������ ���� � ����. ���� ������� �������� ������ ��� ����� ����,
* ��������� ��� ����� ��������������� ��� ��������������.
*/

void r_get_peers(KRPCmsg MSG_q, /* int ip,*/ TIdUDPClient* Client_) // ��� ������ ������ ���� get_peers, �.�. ����� �� num_id
{
	node left_neighbor;
	node right_neighbor;
	KRPCmsg MSG_r;               // ���������, ������� ����� �������� �� ������
	UnicodeString signature = "SIGNATURE";
	size_t tbslen;               // ������ ������������� ������
	strlist NIS;
	// int i = 0;
	compare_result cmp;// ��� �� �� ����� ��� ������ �� ������� ������
	int lvl = MSG_q.a.lvl; 		 // ������� ����-������, �� ������� ��������� ����� � ������ ������.
								 // � ������ ���� ������ �� ���������, ��� -1.
	node_type typenode = peer;
	if (MSG_q.recipient.num_id != "NI" && MSG_q.recipient.num_id != nif->self_info.num_id)   // ���� num_id �������� �� ��������� � �����,
		if (FileExists(GetCurrentDir() + "\\tx\\" + MSG_q.recipient.num_id + ".txt")){      // ���� ����� � ������� ����
			NIS->LoadFromFile(GetCurrentDir() + "\\tx\\" + MSG_q.recipient.num_id + ".txt"); // � ������� ����������������
			typenode = tx;                                                              // ����� ��� ����������.
			}
		else if (FileExists(GetCurrentDir() + "\\blk\\" + MSG_q.recipient.num_id + ".txt")){
			NIS->LoadFromFile(GetCurrentDir() + "\\blk\\" + MSG_q.recipient.num_id + ".txt");
			typenode = blk;
			}
		else
		{
			ShowMessage("Required ID not found in stored blocks and transactions!");
			return;
		}

	node_info* INFO;
	INFO = (typenode == peer) ? nif : (new node_info(NIS));
	cmp = compare_addr(MSG_q.recipient.num_id, MSG_q.a.info_hash);
	if (cmp == equally) 										// ���� num_id ���, �������� ��������� (��������, ����������)
		{														// ����� �������� ����,
			MSG_r.r.values = MSG_q.recipient; 					// �� �� ��������� id �������� � ������������ ����������,
			Client_->Host = make_ip(MSG_q.a.start_node.ip->Strings[0]);		// ���������� ����� �� �������� ������ ���������� ������,
																// � ���������� ���� ������ ����������
			MSG_r.kid = MSG_q.kid;
			MSG_r.y = r;
			MSG_r.q = get_peers;
			Client_->Send(BEncode(MSG_r));
		}
							// proof_of_search �� ����������, �.�. ��� ����� ��� ��� ��������. ��� �������� ������� ��������� � �������.
	else 					// ����� ���������, ������ �� ��� �������� �������� ����
		{ // ������� ������ ��� �����										// ���� ������, �� ��� ���� �� ������ �� �����.
		while (lvl >= 0) 													// ���������� �� �������, ���� ����� ������ ��� ����� ��
		{ 																	// ������ ������.
																			// ���������� ��������� ��� � id ������� ������
			if(compare_addr(INFO->neighbours_info[lvl][cmp == right].num_id, MSG_q.a.info_hash) == cmp)
			{   															// ���� ��� ������ �� ���, ��� �����, �� ���� ��������
				Client_->Host = make_ip(INFO->neighbours_info[lvl][cmp == right].ip->Strings[0]);  		// ������ �� ������� ������� ������
				MSG_r.a.info_hash = MSG_q.a.info_hash;                      // ���������� ������ �� ip-�������, ������������� ��
				MSG_r.a.start_node.ip = MSG_q.a.start_node.ip;              // ������� ������
				MSG_r.a.start_node.name_id = MSG_r.a.start_node.num_id = MSG_q.a.start_node.num_id;      // TODO: �� ����� ���� ��� ����� ������� ���������
				MSG_r.a.lvl = lvl;                                          // � ��������� ����������.

				MSG_r.kid = MSG_q.kid;                                      // � ���� �������� ������� �� ����� ������ ��� ������ ����� -
				MSG_r.y = q;                                                // �������������, �.�. ������ ���� ������ �������������
				MSG_r.q = get_peers;
				MSG_r.sender = INFO->self_info;
				MSG_r.recipient = INFO->neighbours_info[lvl][cmp == right]; 	// ������ ���� ��������� ����� ������
																			// ����� ������ �� EVP_PKEY � �������� ��� � proof_of_search
/*				EVP_PKEY_CTX *ctx;
				ctx = EVP_PKEY_CTX_new(SelfEVP.get(), NULL);				// ������ DSA ��������, ����������� � EVP.
				if (!ctx)
					ShowMessage("get_peers: Failed to create sign context.");

				if (EVP_PKEY_sign_init(ctx) <= 0)                       	// �������������� ���
					ShowMessage("get_peers: Failed to initialize sign context.");

				if (EVP_PKEY_sign(ctx, NULL, &siglen, INFO->neighbours_info[lvl][1].num_id.c_str(), 40) <= 0)
					ShowMessage("get_peers: Failed to determine memory size."); // ��� ������� ����� � siglen ������ ������,
																				// ����������� ��� �������� ������� ��������� �� INFO
				signature = OPENSSL_malloc(siglen);								//  �� ���������� ctx. ������ �������� ������.

				if (!sig) ShowMessage("get_peers: OpenSSL malloc failure.");
				if (EVP_PKEY_sign(ctx, signature, &siglen, INFO->neighbours_info[lvl][1].num_id.c_str(), 40) <= 0)
					ShowMessage("get_peers: sign failure.");	 // ��� ������� ����������� ������ � 4-� ���������,
						   // ���������� ������� � ����� char* signature, � ���������� ���������� ���� - � siglen     */
				MSG_r.proof_of_search = MSG_q.proof_of_search + signature;	// ������ �������. �������� ����?

				MSG_r.SendKRPCmsg(Client_);
				return;
			}
			else lvl--; // ���� ��� ������ ��� ����� id ������, ���� ���������� �� 1 ������� ����.
		}   														// ���� �� ����� �� ����� �����, �.�. lvl == -1, �� �� ��� �����
																	// �������������, �������� ���� �� ����������, � ���� ���������
		MSG_r.r.nodes.resize(2);		                        // ���������� ���� �� ��� � ������� ������ �� 0-� ������.
		MSG_r.r.nodes[1] = cmp == right ? INFO->neighbours_info[lvl][1] : INFO->self_info;	// ���� �� �������� ������, �� ������ ����� ��� �����, � ����� ��.
		MSG_r.r.nodes[0] = cmp == right ? INFO->self_info : INFO->neighbours_info[lvl][0];	// ���� �����, �� ��������.

		MSG_r.kid = MSG_q.kid;
		MSG_r.y = r;
		MSG_r.q = get_peers;
		MSG_r.SendKRPCmsg(Client_);
		}

}


/* � ���� ������� ����������� ��������� ������� (start_node) ����� ���� ���� ���� (���),
 * ������������ ����� (announced_peer) - ������ (��������, ��-���), � ������������ ����������, �� ������� �� ��������� - ������.
 * ������� �� ������.
 */

void r_announce_peer(KRPCmsg MSG_q,/* int ip,*/ TIdUDPClient* Client_)
{
	strlist NIS;
	node_type typenode = peer;
	if (MSG_q.recipient.num_id != nif_s->Strings[0].SubString(1,20))   			// ���� num_id �������� �� ��������� � �����,
		if (FileExists(GetCurrentDir() + "\\tx\\" + MSG_q.recipient.num_id + ".txt"))       // ���� ����� � ������� ����
		{
		NIS->LoadFromFile(GetCurrentDir() + "\\tx\\" + MSG_q.recipient.num_id + ".txt"); // � ������� ����������������
		typenode = tx;
		}
		else if (FileExists(GetCurrentDir() + "\\blk\\" + MSG_q.recipient.num_id + ".txt")) // ����� ��� ����������.
		{
		NIS->LoadFromFile(GetCurrentDir() + "\\blk\\" + MSG_q.recipient.num_id + ".txt");
		typenode = blk;
		}
		else
		{
		ShowMessage("Required ID not found in stored blocks and transactions!");
		return;
		}
	node_info* INFO;
	INFO = (typenode == peer) ? nif : (new node_info(NIS));

	compare_result cmp = compare_addr(INFO->self_info.num_id, MSG_q.sender.num_id);    // ������ ��� ����� ��������� �����������?
							// ���� ��� left, �.�. 0 - ������, �� ������ � �� �����.
	int L;
													// ����� �������� num_id ����������� ���������
	int cpref = common_prefix(INFO->self_info.name_id, MSG_q.a.announced_node.name_id);
	if (compare_addr(MSG_q.a.announced_node.num_id, MSG_q.sender.num_id) == equally)       // � num_id ������������� ����.
	{                          	// ���� ��������� ������ �� ���� ������������� ���� - ���������� ��� � ������� ������ ������.

		for (L = MSG_q.a.lvl; L <= cpref; L++)
				 // �� ������ �������� ��� ����� ������� �� ������� �� lvl-�� �� ����� ������ �������� ������������.
			INFO->neighbours_info[L][cmp] = MSG_q.sender;

		(INFO->ToStringList())->SaveToFile(typenode == tx ? (GetCurrentDir() + "\\tx\\" + MSG_q.recipient.num_id + ".txt") :
										(typenode == blk ? (GetCurrentDir() + "\\blk\\" + MSG_q.recipient.num_id + ".txt") :
															 (GetCurrentDir() + "\\" + nif_filename)));
		L--;							  // ����� ����� ����� L �� 1 ������ ��� �����.
		KRPCmsg MSG_r;                    // ������� ���������� �� ������� ������ � ������ �������.

		if (INFO->neighbours_info[L][1 - cmp].num_id == (cmp == left ? "***" : "~~~")) { 		// ���� �� �� ���� ������ � ����, �����
			KRPCmsg MSG_r;														// �������� ����������,
			MSG_r.y = r;								 						// � ��������� ������ �� ����������.
			MSG_r.q = announce_peer;

			MSG_r.kid = MSG_q.kid;
			MSG_r.sender = INFO->self_info;
			MSG_r.recipient = MSG_q.a.start_node;
			MSG_r.r.values = (cmp == left ? RIGHT_EDGE : LEFT_EDGE);
			MSG_r.r.lvl = cpref;                                                // � ���� ������ ������� �� ����� �����, �.�. ����� ������� ���������� � ��������������
                                                                                // ����� -1, � �� cpref.
			MSG_r.SendKRPCmsg(Client_);
		}
		else {
			KRPCmsg MSG_q;	                                                    // ���� �� � ����, ������� ������ ������
			MSG_q.y = q;                                                        // � ������ �������.
			MSG_q.q = announce_peer;
			MSG_q.a.announced_node = MSG_q.a.announced_node;
			MSG_q.a.start_node = MSG_q.a.start_node;
			MSG_q.a.lvl = cpref;                                                // "������ �������� ���� ������� �� lvl-��".

			MSG_q.kid = MSG_q.kid;
			MSG_q.sender = INFO->self_info;
			MSG_q.recipient = INFO->neighbours_info[L][1 - cmp];                  // ������ ������� - ��������������� ���,
																				// ������ ������ ���������.
			MSG_q.SendKRPCmsg(Client_);
		}
	}
	else   // � ������ ���� ��������� ������ �� �� ���� ������������� ���� - �� �� ������ � �������, � ������ ������� lvl-�������� name_id.
	{	if (common_prefix(INFO->self_info.name_id, MSG_q.a.announced_node.name_id) > MSG_q.a.lvl){ // � ������ ���������� ����� ��������� ��
			KRPCmsg MSG_r;														// ���� ���������� �������, �� ������ �� ������,
			MSG_r.y = r;								 						// � ��������� ������ �� ����������.
			MSG_r.q = announce_peer;                                           	// ������� ��� ����� �������,
																				// �� ��� �������� ��� ������ ������ �� ����������.
			MSG_r.kid = MSG_q.kid;                                              // �� ����� ������� ��������� ��� � ������,
			MSG_r.sender = MSG_r.r.values = INFO->self_info;                    // ����-��������� ������ ���, ������ ��� name_id.
			MSG_r.recipient = MSG_q.a.start_node;                                 // TODO �� �����: ������ ��� �� ������ ��������
			MSG_r.r.lvl = cpref;
																				// ���������� ����� �� ����� �������, ������� ��
			MSG_q.SendKRPCmsg(Client_);				                       	// ������ ��������� � ��� ��������� � ������ �������.
		}
		else{
			if (INFO->neighbours_info[L][cmp].num_id == (cmp == left ? "~~~" : "***")) { 		// ���� �� �� ���� ������ � ����, �����
				KRPCmsg MSG_r;														// �������� ����������,
				MSG_r.y = r;								 						// � ��������� ������ �� ����������.
				MSG_r.q = announce_peer;

				MSG_r.kid = MSG_q.kid;
				MSG_r.sender = INFO->self_info;
				MSG_r.recipient = MSG_q.a.start_node;
				MSG_r.r.values = (cmp == left ? LEFT_EDGE : RIGHT_EDGE);

				Client_->Host = make_ip(MSG_q.a.start_node.ip->Strings[0]);
				Client_->Send(BEncode(MSG_r));
			}
			else{															 	// ���� �� � ����, ������� ������.
				KRPCmsg MSG_r;
				MSG_r.y = q;

				MSG_r.q = announce_peer;
				MSG_r.a.announced_node = MSG_q.a.announced_node;
				MSG_r.a.start_node = MSG_q.a.start_node;

				MSG_r.kid = MSG_q.kid;
				MSG_r.sender = INFO->self_info;
				MSG_r.recipient = INFO->neighbours_info[L][cmp];

				Client_->Host = make_ip(INFO->neighbours_info[L][cmp].ip->Strings[0]);
				Client_->Send(BEncode(MSG_r));
			}
		}

	}
}

void process_query(KRPCmsg* M, TIdUDPClient* Client_){  // ���������� ��� ��������� �������
	switch (M->q) {
	case ping: 					// todo
		break;                  // TODO: ����� �� ����������� ���������� KRPC ���������
	case get_peers:				// �� ������ ��� ��������� ����� �� ��������� �����������.
		r_get_peers(*M, Client_);
		break;
	case announce_peer:
		r_announce_peer(*M, Client_);
		break;
	}
}
 /*
KRPCmsg r_ask_ip(KRPCmsg MSG_q, int ip, TIdUDPClient* Client_)
{
	KRPCmsg MSG_r;
	Client_->Host = make_ip(ip);
	MSG_r.sender = SELF_INFO->self_info;
	MSG_r.r.nodes += ip;
	MSG_r.kid = "||KID__KID__KID||";
	MSG_r.y = (CMD)1;
	MSG_r.q = (type_query)4;

	return BEncode(MSG_r);
}                 */
