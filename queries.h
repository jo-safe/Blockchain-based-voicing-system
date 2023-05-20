#pragma once

//------------------------------------------------------------------------------
UnicodeString make_ip(UnicodeString ip);

void init1(node newnode, TIdUDPClient* Client_)   // ����� ������� ������ ���� � ���� �����
										// ispeer ����������, ����������� �� ���, ��� �� ����������� ����������/����.
{	//if(ispeer)                          // ������ � ����� ���� ��� ������ ���� ������� �� ��� �������� �������� �����.
	  //	nif_s->Add(R->r.values);  	// ������� ����������/����� ���������� � ��������� �������,
									// ����� �������������� ���� t �������� ���������� �� ���������. � ����� �� ��� ������� ��������
									// ������� �����������, ������� ������ � ����� ���������� � ���� ���������� �����.
   //	tif_s->Add(R->r[0].values);	// �� ����� num_id ����������, �������� ��� ��, ����� �������, search_proof ������� �����������.
									// name_id �� ����� ����� ������ ���������� �����.

									// � ������ ������� ���������� init1 ���������� �� (TODO) ������� ���������� ����������, � �������
									// ���������� ������������ �� process_request ����� ���������� ���������� ���������. � ���� �������
									// ����������� id ���������� � ���������� � init1 � �������� r[0].values � ������� NumidNameidIP.
									// � ������ ������� ���������� ��� ���� ��������� ������������ ���������.
									// ����� �� ���������� ���� ���������� id � ������ �����. � ������ ���� �� ��������� ����������,
									// ����� �������� ���������� � ��� � ���� ����������.

	KRPCmsg temp; 					// ������ ������ get_peers �� ����� numid ������������ ����
	temp.kid = kid_counter++;
	temp.y = q;
	temp.q = get_peers;
	temp.sender = newnode; 		// ����������� ���� new_node - ����������� ���������, �� ���� ���� num_id.

	// temp.recipient.num_id = temp.recipient.name_id = snf_s->Strings[0].SubString(1,20);	// ���������� ���� �������������
	temp.recipient.ip->Strings[0] = snf_s->Strings[0].SubString(1,10);
											// TODO: �������, ���, ���� ������ ���� ������������� �� ��������, ���������� �������
	temp.a.lvl = 0; 						// ���� � ������ 0
	temp.a.info_hash = newnode.num_id; 		// ��� ������? numid ������������ ����!
	temp.a.start_node = nif->self_info;      // �� - ���������� ������. ������ ��, � �� ����������, � ������ ���������� ����������.
	temp.isanswered = false;

	temp.SendKRPCmsg(Client_);

	request temprq;							// ������ ����������, ������� ����� �������� ���������� ������� init2.
	temprq.name = "init2";
	temprq.q.reserve(1);          			// � ���������� ���� ������.
	temprq.r.reserve(1);          			// �������� ������ ��� ���� �����.
	temprq.count = 1;
	temprq.q.push_back(temp);                     // ��� � ���������� ������������ ������.

	req.push_back(temprq);
}

void init2(request R, TIdUDPClient* Client_)
{   // ����������, ���������� � ��� �������, �������� � ���� nodes ������ ������������� �������
	// ������ �� ���� �����, ����� �������� ���� �� 0-� ������ �������� ����� ���� � num_id R.q[0].a.info_hash.
	// �� ������ ��������� ���� ����� ������ ������� announce_peer �� ���������� � �������� �����, � ����� �������� �� ����.

	strlist NIS = new TStringList;				// ������� ���� �������� ���� ��� ������������ ����. num_id ��� ������� � info_hash ������� �� �����.
	node_type typenode = peer;	// �������� �� ����������� ���� �����?
	if (R.q[0].a.info_hash != nif->self_info.num_id)   							// ���� num_id ������������ ���� �� ��������� � �����,
		if (FileExists(GetCurrentDir() + "\\tx\\" + R.q[0].a.info_hash + ".txt")){      // ���� ����� � ������� ����
			NIS->LoadFromFile(GetCurrentDir() + "\\tx\\" + R.q[0].a.info_hash + ".txt"); // � ������� ����������������
			typenode = tx;                                                                // ����� ��� ����������.
			}
		else if (FileExists(GetCurrentDir() + "\\blk\\" + R.q[0].a.info_hash + ".txt")){
			NIS->LoadFromFile(GetCurrentDir() + "\\blk\\" + R.q[0].a.info_hash + ".txt");
			typenode = blk;
			}
		else
		{
			ShowMessage("Required ID not found in stored blocks and transactions!");
			return;
		}                                                                       // � ������ ���������� ���������� ������ ����� nif.
																				// ��������� �� ����� ������������ ����.
	node_info* INFO;                                                            // ������ ���������� � ����������� � �������
	INFO = (typenode == peer) ? nif : (new node_info(NIS));								// �� ���� �������.
	INFO->self_info = R.q[0].sender;

	int cpref_left, cpref_right;		// ����� �������� name_id � ����� � ������ ��������.
                                        // ���� ����� ����� ������� -1 � ����� ������ �������.
	cpref_left 	= common_prefix(INFO->self_info.name_id, R.r[0].r.nodes[0].name_id);
	cpref_right = common_prefix(INFO->self_info.name_id, R.r[0].r.nodes[1].name_id);

	INFO->neighbours_info.resize(cpref_left > cpref_right ? cpref_left : cpref_right);
	for (unsigned int  i = 0; i < INFO->neighbours_info.size(); i++)	// ����� ����� ������� �� ������������ ����� ������� � ������ � ����� ��������.
		INFO->neighbours_info[i].resize(2);				// �� ������ �� ������� ������ ���� �� ���� �������.
								// ��� ������� �� ���� ������� ����� �������� ��� �� ���� ������ � ������������ � ���������
								// �������.
	for (int j = 0; j <= cpref_left; j++){ 					// ��������� ������ ������.
		INFO->neighbours_info[j][0] = R.r[0].r.nodes[0];
		if (cpref_right == -1)
			INFO->neighbours_info[j][1] = RIGHT_EDGE;        // ���� ������ ����, ����� ��� ������������ � ����� ������� �� ��� ������.
	}

	for (int j = 0; j <= cpref_right; j++){					// ��������� ������� ������.
		INFO->neighbours_info[j][1] = R.r[0].r.nodes[1];
		if (cpref_left == -1)                               // ���������� ���� ����� ����.
			INFO->neighbours_info[j][1] = LEFT_EDGE;
	}
	// � ���� ������ � ������� ������ �� ������� �� ������� ����� ������� ��������� ����� ���������� �������������������� ��������!!!
	// ��� ���������������� ����� ��� ����� ��������� ������ �� announce_peer. ������ ����� ��������� ������� � ��� �������.

	(INFO->ToStringList())->SaveToFile(typenode == tx ? (GetCurrentDir() + "\\tx\\" + R.q[0].a.info_hash + ".txt") :
										(typenode == blk ? (GetCurrentDir() + "\\blk\\" + R.q[0].a.info_hash + ".txt") :
															 (GetCurrentDir() + "\\" + nif_filename)));

	KRPCmsg left_msg, right_msg;                	// ����� ��� �������� ���������� ��������� �� ��, � ����� �����������
	bool to_left_right [2];							// ����� ��������� ���������.
	to_left_right[0] = (INFO->neighbours_info[cpref_left][0].num_id != "***") && (INFO->neighbours_info[cpref_left][0].num_id != "~~~");
	to_left_right[1] = (INFO->neighbours_info[cpref_right][1].num_id != "***") &&(INFO->neighbours_info[cpref_right][1].num_id != "~~~");
	if (to_left_right[0]) {
		left_msg.kid = kid_counter++;
		left_msg.y = q;
		left_msg.q = announce_peer;
		left_msg.a.announced_node = INFO->self_info;
				// TODO �� �����: ��������� ������������ � ����� ������� ����������� proof_of_search
		left_msg.a.start_node = nif->self_info;	// ��������� ������� - ��, ��� ���.
		left_msg.a.lvl = 0;        	              // "������ ���� ������� ������� � 0-�� ������".
		left_msg.sender = INFO->self_info;
		left_msg.recipient = INFO->neighbours_info[cpref_left][0];
		left_msg.isanswered = false;

		left_msg.SendKRPCmsg(Client_);
	}
	if (to_left_right[1]) {
		right_msg.kid = kid_counter++;
		right_msg.y = q;
		right_msg.q = announce_peer;
		right_msg.a.announced_node = INFO->self_info;
				// TODO �� �����: ��������� ������������ � ����� ������� ����������� proof_of_search
		right_msg.a.start_node = nif->self_info;	// ��������� ������� - ��, ��� ���.
		right_msg.a.lvl = 0;
		right_msg.sender = INFO->self_info;
		right_msg.recipient = INFO->neighbours_info[cpref_right][1];
		right_msg.isanswered = false;

		right_msg.SendKRPCmsg(Client_);
	}

	request temprq;							// ������ ����������, ������� ����� �������� ���������� ������� init3.
	temprq.name = "init3";
	temprq.count = to_left_right[0] && to_left_right[1] ?  2 : (to_left_right[0] || to_left_right[1] ? 1 : 0);
			// � ���������� 0, 1 ��� 2 ������� � ����������� �� ����, ������� ��������� ����������.
	temprq.q.reserve(temprq.count);
	temprq.r.reserve(temprq.count);			// �������� ������.
	if (to_left_right[0])                   // ���������� � ���������� �������.
			temprq.q.push_back(left_msg);   // ������ ������ ��� �����.
	if (to_left_right[1])
			temprq.q.push_back(right_msg);
	temprq.answered_count = 0;				// ���� ������� �� ������.

	req.push_back(temprq);
}

void init3 (request R, TIdUDPClient* Client_)
{
	strlist NIS = new TStringList;				// ������� ���� �������� ���� ��� ������������ ����. num_id ��� ������� � info_hash ������� �� �����.
	node_type typenode = peer;	// �������� �� ����������� ���� �����?
	if (R.q[0].a.info_hash != nif->self_info.num_id)   							// ���� num_id ������������ ���� �� ��������� � �����,
		if (FileExists(GetCurrentDir() + "\\tx\\" + R.q[0].a.announced_node.num_id + ".txt")){      // ���� ����� � ������� ����
			NIS->LoadFromFile(GetCurrentDir() + "\\tx\\" + R.q[0].a.announced_node.num_id + ".txt"); // � ������� ����������������
			typenode = tx;                                                             // ����� ��� ����������.
			}
		else if (FileExists(GetCurrentDir() + "\\blk\\" + R.q[0].a.announced_node.num_id + ".txt")){
			NIS->LoadFromFile(GetCurrentDir() + "\\blk\\" + R.q[0].a.announced_node.num_id + ".txt");
			typenode = blk;
			}
		else
		{
			ShowMessage("Required ID not found in stored blocks and transactions!");
			return;
		}                                                                       // � ������ ���������� ���������� ������ ����� nif.
																				// ��������� �� ����� ������������ ����.
	node_info* INFO;                                                            // ������ ���������� � ����������� � �������
	INFO = (typenode == peer) ? nif : (new node_info(NIS));							  // �� ���� �������.
																				// ���� ��� 2 ������ ������?
	if (R.count == 1)     // ���� ��� �������� ���� �����, ��� ��������, ��� � ������ ������� ����� ����� ������ ����.
	{   int cpref_former = common_prefix (INFO->self_info.name_id, R.q[0].recipient.name_id);	// ����� ������� �� ������ �������.
		int cpref_new 	 = common_prefix (INFO->self_info.name_id, R.r[0].r.values.name_id);    // ����� ������� � �����.
						  // ���� � � ���� ������� ����� ����� - ����, �� ��� �����.
		if (cpref_new == -1)
			return;
		else {            // ����� ��� ������ ������ �����?
			compare_result neighbour_pos = compare_addr(INFO->self_info.num_id, R.r[0].r.values.num_id);
			for (int i = cpref_former + 1; i <= cpref_new; i++)
			{
			INFO->neighbours_info[i][neighbour_pos] = R.r[0].r.values;   // � ��� �������, ������ ������, ����� ����������� ������.
			INFO->neighbours_info[i][1 - neighbour_pos] = (neighbour_pos == left ? RIGHT_EDGE : LEFT_EDGE);
			}             // � ������ ������� ����� ����.
		(INFO->ToStringList())->SaveToFile(typenode == tx ? (GetCurrentDir() + "\\tx\\" + R.q[0].recipient.num_id + ".txt") :
										(typenode == blk ? (GetCurrentDir() + "\\blk\\" + R.q[0].recipient.num_id + ".txt") :
															 (GetCurrentDir() + "\\" + nif_filename)));
			KRPCmsg MSG_q;
			MSG_q.y = q;
			MSG_q.q = announce_peer;
			MSG_q.a.announced_node = INFO->self_info;
			MSG_q.a.start_node = nif->self_info;

			MSG_q.kid = kid_counter++;
			MSG_q.sender = INFO->self_info;
			MSG_q.recipient = R.r[0].sender;

			MSG_q.SendKRPCmsg(Client_);

			request temprq;
			temprq.name = "init3";
	       	temprq.q.resize(1);
	       	temprq.q[0] = MSG_q;
	       	temprq.r.reserve(1);
			temprq.count = 1;
			temprq.answered_count = 0;
            req.push_back(temprq);
		}
	}
	if (R.count == 2)
	{
		compare_result neighbour_pos [2] = { compare_addr(INFO->self_info.num_id, R.r[0].r.values.num_id),	// ����� ��� ������ ������
											 compare_addr(INFO->self_info.num_id, R.r[1].r.values.num_id) };	// ������ �� �������?

		int cpref_former [2] = {	common_prefix(INFO->self_info.name_id, R.q[0].recipient.name_id),
									common_prefix(INFO->self_info.name_id, R.q[1].recipient.name_id) };

		int cpref_new [2] 	 = {    common_prefix(INFO->self_info.name_id, R.r[0].r.values.name_id),
									common_prefix(INFO->self_info.name_id, R.r[1].r.values.name_id) };

		int former_size = INFO->neighbours_info.size();
		INFO->neighbours_info.resize((cpref_new[0] > cpref_new[1] ? cpref_new[0] : cpref_new[1]) + 1);
		for (unsigned int  i = former_size; i < INFO->neighbours_info.size(); i++)
			INFO->neighbours_info[i].resize(2);
		// ��������� ������� �������, ������� ������������ ����� ����� ��������� � ������ ��������, � �� ������ �� ����� �������
		// ������ ���� ��� ���� �������.
		if (R.r[0].r.values.num_id == "***" || R.r[0].r.values.num_id == "~~~")
			if (R.r[1].r.values.num_id == "***" || R.r[1].r.values.num_id == "~~~") // ����� ����, ������ ����.
				return;
			else {
				for (int i = cpref_former[1] + 1; i < cpref_new[1]; i++)			// ����� ����, ������ �� ����.
					 INFO->neighbours_info[i][1] = R.r[1].r.values;             // ����� ����������� ������ ������.
				for (int i = cpref_former[0] + 1; i < cpref_new[1]; i++)        // ����� ���� ����� �� ������ �������� � ������ �������.
					 INFO->neighbours_info[i][0] = LEFT_EDGE;
				}

		else if (R.r[1].r.values.num_id == "***" || R.r[1].r.values.num_id == "~~~"){  	// ����� �� ����, ������ ����.
				for (int i = cpref_former[0] + 1; i < cpref_new[0]; i++)         // ����� ����������� ������ �����.
					INFO->neighbours_info[i][0] = R.r[0].r.values;              // ����� ���� ������ �� ������ �������� � ����� �������.
				for (int i = cpref_former[1] + 1; i < cpref_new[0]; i++)
					INFO->neighbours_info[i][1] = RIGHT_EDGE;
			}
			 else {                          	                                // ����� �� ����, ������ �� ����.
				for (int i = cpref_former[0] + 1; i < cpref_new[0]; i++)         // ����� ���������� ������� ������ � �����.
					INFO->neighbours_info[i][0] = R.r[0].r.values;
				for (int i = cpref_former[1] + 1; i < cpref_new[1]; i++)
					INFO->neighbours_info[i][1] = R.r[1].r.values;
			}

		(INFO->ToStringList())->SaveToFile(typenode == tx ? (GetCurrentDir() + "\\tx\\" + R.q[0].a.announced_node.num_id + ".txt") :
										(typenode == blk ? (GetCurrentDir() + "\\blk\\" + R.q[0].a.announced_node.num_id + ".txt") :
															 (GetCurrentDir() + "\\" + nif_filename)));

		vector <KRPCmsg> MSG_q;
		int responses_count = 0;
		if (!(R.r[0].r.values.num_id == "~~~" || R.r[0].r.values.num_id == "***"))
		{
			KRPCmsg MSG_q0;
			responses_count++;
			MSG_q0.y = q;
			MSG_q0.q = announce_peer;
			MSG_q0.a.announced_node = INFO->self_info;
			MSG_q0.a.start_node = nif->self_info;

			MSG_q0.kid = kid_counter++;
			MSG_q0.sender = INFO->self_info;
			MSG_q0.recipient = R.r[0].sender;

			MSG_q.push_back(MSG_q0);
			MSG_q0.SendKRPCmsg(Client_);
		}

		if (!(R.r[1].r.values.num_id == "~~~" || R.r[1].r.values.num_id == "***"))
		{
			KRPCmsg MSG_q1;
			responses_count++;
			MSG_q1.y = q;
			MSG_q1.q = announce_peer;
			MSG_q1.a.announced_node = INFO->self_info;
			MSG_q1.a.start_node = nif->self_info;

			MSG_q1.kid = kid_counter++;
			MSG_q1.sender = INFO->self_info;
			MSG_q1.recipient = R.r[1].sender;

			MSG_q.push_back(MSG_q1);
			MSG_q1.SendKRPCmsg(Client_);
		}

		if (responses_count == 0) return;

		request temprq;
		temprq.name = "init3";
		if (responses_count >= 1)
			temprq.q.push_back(MSG_q[0]);
		if (responses_count >= 2)
			temprq.q.push_back(MSG_q[1]);

		temprq.r.reserve(responses_count);										// �������� ����� ��� ������� �������, �������
		temprq.count = responses_count;											// ��������� ��������.

	   	temprq.answered_count = 0;
		req.push_back(temprq);
	}
}
//---------------------------------------------------------------------------

void process_request(request R, TIdUDPClient* Client_)    // ����������, ������������ ��� ��������� ������ �� ��� ������� ����������. � ������
{								   // case ����� �������� ���������� ���������� �� ������� ���������� ����� ���������� �������.
	if (R.name == "init2")
			init2(R, Client_); // initx ��������� �� ���� ����������.

	if (R.name == "init3")
			init3(R,Client_);

};
