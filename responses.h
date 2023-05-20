
// RESPONSES
#pragma once

//static ifstream in1;
//static ofstream out1;

//UnicodeString r_get_peers(KRPCmsg, int, TIdUDPClient*);
//UnicodeString r_announce_peer(KRPCmsg, int, TIdUDPClient*);
//UnicodeString r_ask_ip(KRPCmsg, int, TIdUDPClient*);
//-----------------------------------------------------------------------------
/* Это ответ на сообщение типа get_peers
* В случае если узел новый, мы выдаём ему его адрес в сети и он самостоятельно
* отправляет все запросы, необходимые для интегрирования в сеть.
* В случае если узел отправляет get_peers, он являемся звеном в алгоритме
* поиска узла по адресу, указанному в сообщении. Мы принимаем у него эстафету,
* выполняем поиск, и, при необходимости, передаём поисковый запрос следующему.
* Важная деталь состоит в том, что этот алгоритм не требует проверки на то,
* находится ли данный узел с краю. Если соседом является правый или левый край,
* поисковый хеш будет идентифицирован как несуществующий.
*/

void r_get_peers(KRPCmsg MSG_q, /* int ip,*/ TIdUDPClient* Client_) // нам пришёл запрос типа get_peers, т.е. поиск по num_id
{
	node left_neighbor;
	node right_neighbor;
	KRPCmsg MSG_r;               // сообщение, которое будет реакцией на запрос
	UnicodeString signature = "SIGNATURE";
	size_t tbslen;               // размер подписываемой строки
	strlist NIS;
	// int i = 0;
	compare_result cmp;// идём ли мы влево или вправо по цепочке поиска
	int lvl = MSG_q.a.lvl; 		 // Уровень скип-листов, на котором находится поиск в данный момент.
								 // В случае если запрос не поисковый, это -1.
	node_type typenode = peer;
	if (MSG_q.recipient.num_id != "NI" && MSG_q.recipient.num_id != nif->self_info.num_id)   // если num_id адресата не совпадает с нашим,
		if (FileExists(GetCurrentDir() + "\\tx\\" + MSG_q.recipient.num_id + ".txt")){      // надо найти и открыть файл
			NIS->LoadFromFile(GetCurrentDir() + "\\tx\\" + MSG_q.recipient.num_id + ".txt"); // с данными соответствующего
			typenode = tx;                                                              // блока или транзакции.
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
	if (cmp == equally) 										// Если num_id нас, адресата сообщения (возможно, транзакции)
		{														// равен искомому хешу,
			MSG_r.r.values = MSG_q.recipient; 					// то мы переносим id адресата в возвращаемую переменную,
			Client_->Host = make_ip(MSG_q.a.start_node.ip->Strings[0]);		// выставляем сокет на отправку данных инициатору поиска,
																// и отправляем наши данные инициатору
			MSG_r.kid = MSG_q.kid;
			MSG_r.y = r;
			MSG_r.q = get_peers;
			Client_->Send(BEncode(MSG_r));
		}
							// proof_of_search не изменяется, т.к. наш адрес уже был добавлен. Эту проверку следует перенести в будущем.
	else 					// Иначе проверяем, больше ли хеш адресата искомого хеша
		{ // СМОТРИМ ВПРАВО или ВЛЕВО										// Если меньше, то идём вниз по списку из файла.
		while (lvl >= 0) 													// Спускаемся по уровням, пока сосед справа или слева не
		{ 																	// станет меньше.
																			// сравниваем поисковый хеш с id нужного соседа
			if(compare_addr(INFO->neighbours_info[lvl][cmp == right].num_id, MSG_q.a.info_hash) == cmp)
			{   															// если хеш дальше от нас, чем сосед, то надо передать
				Client_->Host = make_ip(INFO->neighbours_info[lvl][cmp == right].ip->Strings[0]);  		// запрос по цепочке нужному соседу
				MSG_r.a.info_hash = MSG_q.a.info_hash;                      // используем первый из ip-адресов, ответственных за
				MSG_r.a.start_node.ip = MSG_q.a.start_node.ip;              // правого соседа
				MSG_r.a.start_node.name_id = MSG_r.a.start_node.num_id = MSG_q.a.start_node.num_id;      // TODO: на самом деле его нужно сначала пингануть
				MSG_r.a.lvl = lvl;                                          // с созданием требования.

				MSG_r.kid = MSG_q.kid;                                      // В этой ситуации никогда не будет такого что правый сосед -
				MSG_r.y = q;                                                // бесконечность, т.к. нельзя быть дальше бесконечности
				MSG_r.q = get_peers;
				MSG_r.sender = INFO->self_info;
				MSG_r.recipient = INFO->neighbours_info[lvl][cmp == right]; 	// теперь надо подписать адрес соседа
																			// своим ключом из EVP_PKEY и добавить его в proof_of_search
/*				EVP_PKEY_CTX *ctx;
				ctx = EVP_PKEY_CTX_new(SelfEVP.get(), NULL);				// создаём DSA контекст, привязанный к EVP.
				if (!ctx)
					ShowMessage("get_peers: Failed to create sign context.");

				if (EVP_PKEY_sign_init(ctx) <= 0)                       	// инициализируем его
					ShowMessage("get_peers: Failed to initialize sign context.");

				if (EVP_PKEY_sign(ctx, NULL, &siglen, INFO->neighbours_info[lvl][1].num_id.c_str(), 40) <= 0)
					ShowMessage("get_peers: Failed to determine memory size."); // эта команда пишет в siglen размер данных,
																				// необходимых для создания подписи сообщения из INFO
				signature = OPENSSL_malloc(siglen);								//  по параметрам ctx. Теперь выделяем память.

				if (!sig) ShowMessage("get_peers: OpenSSL malloc failure.");
				if (EVP_PKEY_sign(ctx, signature, &siglen, INFO->neighbours_info[lvl][1].num_id.c_str(), 40) <= 0)
					ShowMessage("get_peers: sign failure.");	 // эта команда подписывает строку в 4-м аргументе,
						   // записывает подпись в буфер char* signature, а количество записанных байт - в siglen     */
				MSG_r.proof_of_search = MSG_q.proof_of_search + signature;	// ставим подпись. привести типы?

				MSG_r.SendKRPCmsg(Client_);
				return;
			}
			else lvl--; // Если хеш меньше или равен id соседа, надо спуститься на 1 уровень ниже.
		}   														// Если мы дошли до этого места, т.е. lvl == -1, то на нас поиск
																	// заканчивается, искомого узла не существует, и надо отправить
		MSG_r.r.nodes.resize(2);		                        // инициатору пару из нас и нужного соседа на 0-м уровне.
		MSG_r.r.nodes[1] = cmp == right ? INFO->neighbours_info[lvl][1] : INFO->self_info;	// Если мы смотрели вправо, то правым будет наш сосед, а левым мы.
		MSG_r.r.nodes[0] = cmp == right ? INFO->self_info : INFO->neighbours_info[lvl][0];	// Если влево, то наоборот.

		MSG_r.kid = MSG_q.kid;
		MSG_r.y = r;
		MSG_r.q = get_peers;
		MSG_r.SendKRPCmsg(Client_);
		}

}


/* В этой функции инициатором алгоритма вставки (start_node) может быть один узел (пир),
 * встраиваемым узлом (announced_peer) - другой (возможно, не-пир), а отправителем соообщения, на которое мы реагируем - третий.
 * Следите за руками.
 */

void r_announce_peer(KRPCmsg MSG_q,/* int ip,*/ TIdUDPClient* Client_)
{
	strlist NIS;
	node_type typenode = peer;
	if (MSG_q.recipient.num_id != nif_s->Strings[0].SubString(1,20))   			// Если num_id адресата не совпадает с нашим,
		if (FileExists(GetCurrentDir() + "\\tx\\" + MSG_q.recipient.num_id + ".txt"))       // надо найти и открыть файл
		{
		NIS->LoadFromFile(GetCurrentDir() + "\\tx\\" + MSG_q.recipient.num_id + ".txt"); // с данными соответствующего
		typenode = tx;
		}
		else if (FileExists(GetCurrentDir() + "\\blk\\" + MSG_q.recipient.num_id + ".txt")) // блока или транзакции.
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

	compare_result cmp = compare_addr(INFO->self_info.num_id, MSG_q.sender.num_id);    // Справа или слева находится отправитель?
							// Если это left, т.е. 0 - значит, мы больше и он слева.
	int L;
													// Нужно сравнить num_id отправителя сообщения
	int cpref = common_prefix(INFO->self_info.name_id, MSG_q.a.announced_node.name_id);
	if (compare_addr(MSG_q.a.announced_node.num_id, MSG_q.sender.num_id) == equally)       // с num_id встраиваемого узла.
	{                          	// Если сообщение пришло от лица встраиваемого узла - встраиваем его и передаём запрос дальше.

		for (L = MSG_q.a.lvl; L <= cpref; L++)
				 // Мы должны объявить его нашим соседом на уровнях от lvl-го до длины общего префикса включительно.
			INFO->neighbours_info[L][cmp] = MSG_q.sender;

		(INFO->ToStringList())->SaveToFile(typenode == tx ? (GetCurrentDir() + "\\tx\\" + MSG_q.recipient.num_id + ".txt") :
										(typenode == blk ? (GetCurrentDir() + "\\blk\\" + MSG_q.recipient.num_id + ".txt") :
															 (GetCurrentDir() + "\\" + nif_filename)));
		L--;							  // после этого цикла L на 1 больше чем нужно.
		KRPCmsg MSG_r;                    // Передаём информацию по цепочке соседу с нужной стороны.

		if (INFO->neighbours_info[L][1 - cmp].num_id == (cmp == left ? "***" : "~~~")) { 		// Если мы на этом уровне с краю, нужно
			KRPCmsg MSG_r;														// сообщить инициатору,
			MSG_r.y = r;								 						// и сообщение дальше не передавать.
			MSG_r.q = announce_peer;

			MSG_r.kid = MSG_q.kid;
			MSG_r.sender = INFO->self_info;
			MSG_r.recipient = MSG_q.a.start_node;
			MSG_r.r.values = (cmp == left ? RIGHT_EDGE : LEFT_EDGE);
			MSG_r.r.lvl = cpref;                                                // В этом случае уровень не очень важен, т.к. общий префикс инициатора с бесконечностью
                                                                                // равен -1, а не cpref.
			MSG_r.SendKRPCmsg(Client_);
		}
		else {
			KRPCmsg MSG_q;	                                                    // Если не с краю, передаём запрос соседу
			MSG_q.y = q;                                                        // с нужной стороны.
			MSG_q.q = announce_peer;
			MSG_q.a.announced_node = MSG_q.a.announced_node;
			MSG_q.a.start_node = MSG_q.a.start_node;
			MSG_q.a.lvl = cpref;                                                // "Можешь добавить меня соседом до lvl-го".

			MSG_q.kid = MSG_q.kid;
			MSG_q.sender = INFO->self_info;
			MSG_q.recipient = INFO->neighbours_info[L][1 - cmp];                  // Нужная сторона - противоположная той,
																				// откуда пришло сообщение.
			MSG_q.SendKRPCmsg(Client_);
		}
	}
	else   // В случае если сообщение пришло не от лица встраиваемого узла - мы не первые в цепочке, и должны сверить lvl-префиксы name_id.
	{	if (common_prefix(INFO->self_info.name_id, MSG_q.a.announced_node.name_id) > MSG_q.a.lvl){ // В случае совпадения нужно уведомить об
			KRPCmsg MSG_r;														// этом инициатора вставки, но ничего не делать,
			MSG_r.y = r;								 						// и сообщение дальше не передавать.
			MSG_r.q = announce_peer;                                           	// Добавив нас своим соседом,
																				// он сам отправит нам прямой запрос на добавление.
			MSG_r.kid = MSG_q.kid;                                              // На каких уровнях добавлять нас в соседи,
			MSG_r.sender = MSG_r.r.values = INFO->self_info;                    // узел-инициатор узнает сам, увидев наш name_id.
			MSG_r.recipient = MSG_q.a.start_node;                                 // TODO на потом: каждый раз мы должны выдавать
			MSG_r.r.lvl = cpref;
																				// инициатору токен на право вставки, который он
			MSG_q.SendKRPCmsg(Client_);				                       	// должен подписать и нам отправить в прямом запросе.
		}
		else{
			if (INFO->neighbours_info[L][cmp].num_id == (cmp == left ? "~~~" : "***")) { 		// Если мы на этом уровне с краю, нужно
				KRPCmsg MSG_r;														// сообщить инициатору,
				MSG_r.y = r;								 						// и сообщение дальше не передавать.
				MSG_r.q = announce_peer;

				MSG_r.kid = MSG_q.kid;
				MSG_r.sender = INFO->self_info;
				MSG_r.recipient = MSG_q.a.start_node;
				MSG_r.r.values = (cmp == left ? LEFT_EDGE : RIGHT_EDGE);

				Client_->Host = make_ip(MSG_q.a.start_node.ip->Strings[0]);
				Client_->Send(BEncode(MSG_r));
			}
			else{															 	// Если не с краю, передаём дальше.
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

void process_query(KRPCmsg* M, TIdUDPClient* Client_){  // обработчик при получении запроса
	switch (M->q) {
	case ping: 					// todo
		break;                  // TODO: нужно по возможности передавать KRPC сообщения
	case get_peers:				// по ссылке или указателю чтобы не вызывался конструктор.
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
