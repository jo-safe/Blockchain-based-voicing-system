#pragma once

//------------------------------------------------------------------------------
UnicodeString make_ip(UnicodeString ip);

void init1(node newnode, TIdUDPClient* Client_)   // поиск соседей нового узла в скип графе
										// ispeer определяет, добавляется ли пир, или же добавляется транзакция/блок.
{	//if(ispeer)                          // данные о новом узле уже должны быть внесены во все локально хранимые файлы.
	  //	nif_s->Add(R->r.values);  	// Вставка транзакции/блока происходит в последнюю очередь,
									// после удовлетворения всех t запросов требования на валидацию. В ответ на эти запросы приходят
									// подписи валидаторов, которые вместе с нашей включаются в поле транзакции сигма.
   //	tif_s->Add(R->r[0].values);	// Мы узнаём num_id транзакции, вычисляя хеш от, среди прочего, search_proof поисков валидаторов.
									// name_id мы узнаём после поиска последнего блока.

									// В случае вставки транзакции init1 вызывается из (TODO) функции добавления транзакции, в которую
									// управление возвращается из process_request после выполнения требования валидации. В этой функции
									// вычисляются id транзакции и передаются в init1 в значении r[0].values в формате NumidNameidIP.
									// В случае вставки транзакции она сама считается отправителем сообщения.
									// Здесь мы записываем наши полученные id в список строк. В случае если мы добавляем транзакцию,
									// нужно добавить информацию о ней в базу транзакций.

	KRPCmsg temp; 					// создаём запрос get_peers на поиск numid вставляемого узла
	temp.kid = kid_counter++;
	temp.y = q;
	temp.q = get_peers;
	temp.sender = newnode; 		// Вставляемый узел new_node - отправитель сообщения, он ищет свой num_id.

	// temp.recipient.num_id = temp.recipient.name_id = snf_s->Strings[0].SubString(1,20);	// отправляем узлу инициализации
	temp.recipient.ip->Strings[0] = snf_s->Strings[0].SubString(1,10);
											// TODO: сделать, что, если первый узел инициализации не отвечает, отправлять второму
	temp.a.lvl = 0; 						// ищем с уровня 0
	temp.a.info_hash = newnode.num_id; 		// Что искать? numid добавляемого узла!
	temp.a.start_node = nif->self_info;      // Мы - инициаторы поиска. Именно мы, а не транзакция, в случае добавления транзакции.
	temp.isanswered = false;

	temp.SendKRPCmsg(Client_);

	request temprq;							// Создаём требование, которое потом передаст управление функции init2.
	temprq.name = "init2";
	temprq.q.reserve(1);          			// В требовании один запрос.
	temprq.r.reserve(1);          			// Выделяем память под один ответ.
	temprq.count = 1;
	temprq.q.push_back(temp);                     // Суём в требование отправляемый запрос.

	req.push_back(temprq);
}

void init2(request R, TIdUDPClient* Client_)
{   // Требование, приходящее в эту функцию, содержит в поле nodes своего единственного запроса
	// массив из двух узлов, между которыми надо на 0-м уровне вставить новый узел с num_id R.q[0].a.info_hash.
	// Мы должны отправить этим узлам прямые запросы announce_peer на добавление в адресную книгу, а также добавить их сами.

	strlist NIS = new TStringList;				// Сначала ищем адресный файл для вставляемого узла. num_id был записан в info_hash запроса на поиск.
	node_type typenode = peer;	// Является ли вставляемый узел пиром?
	if (R.q[0].a.info_hash != nif->self_info.num_id)   							// Если num_id вставляемого узла не совпадает с нашим,
		if (FileExists(GetCurrentDir() + "\\tx\\" + R.q[0].a.info_hash + ".txt")){      // надо найти и открыть файл
			NIS->LoadFromFile(GetCurrentDir() + "\\tx\\" + R.q[0].a.info_hash + ".txt"); // с данными соответствующего
			typenode = tx;                                                                // блока или транзакции.
			}
		else if (FileExists(GetCurrentDir() + "\\blk\\" + R.q[0].a.info_hash + ".txt")){
			NIS->LoadFromFile(GetCurrentDir() + "\\blk\\" + R.q[0].a.info_hash + ".txt");
			typenode = blk;
			}
		else
		{
			ShowMessage("Required ID not found in stored blocks and transactions!");
			return;
		}                                                                       // В случае совпадения используем данные файла nif.
																				// Действуем от имени вставляемого узла.
	node_info* INFO;                                                            // Создаём переменную с информацией о соседях
	INFO = (typenode == peer) ? nif : (new node_info(NIS));								// на всех уровнях.
	INFO->self_info = R.q[0].sender;

	int cpref_left, cpref_right;		// Общие префиксы name_id с левым и правым соседями.
                                        // Края имеют общий префикс -1 с любым другим адресом.
	cpref_left 	= common_prefix(INFO->self_info.name_id, R.r[0].r.nodes[0].name_id);
	cpref_right = common_prefix(INFO->self_info.name_id, R.r[0].r.nodes[1].name_id);

	INFO->neighbours_info.resize(cpref_left > cpref_right ? cpref_left : cpref_right);
	for (unsigned int  i = 0; i < INFO->neighbours_info.size(); i++)	// Задаём число уровней на максимальный общий префикс с правым и левым соседями.
		INFO->neighbours_info[i].resize(2);				// На каждом из уровней создаём пару из двух соседей.
								// Для каждого из двух соседей нужно добавить его на всех нужных в соответствии с префиксом
								// уровнях.
	for (int j = 0; j <= cpref_left; j++){ 					// Добавляем левого соседа.
		INFO->neighbours_info[j][0] = R.r[0].r.nodes[0];
		if (cpref_right == -1)
			INFO->neighbours_info[j][1] = RIGHT_EDGE;        // Если справа край, пишем его одновременно с левым соседом на эти уровни.
	}

	for (int j = 0; j <= cpref_right; j++){					// Добавляем правого соседа.
		INFO->neighbours_info[j][1] = R.r[0].r.nodes[1];
		if (cpref_left == -1)                               // Аналогично если слева край.
			INFO->neighbours_info[j][1] = LEFT_EDGE;
	}
	// В этот момент в ячейках одного из соседей на уровнях между длинами префиксов могут находиться неинициализированные значения!!!
	// Они инициализируются когда нам будут приходить ответы на announce_peer. Теперь нужно составить запросы в обе стороны.

	(INFO->ToStringList())->SaveToFile(typenode == tx ? (GetCurrentDir() + "\\tx\\" + R.q[0].a.info_hash + ".txt") :
										(typenode == blk ? (GetCurrentDir() + "\\blk\\" + R.q[0].a.info_hash + ".txt") :
															 (GetCurrentDir() + "\\" + nif_filename)));

	KRPCmsg left_msg, right_msg;                	// Флаги для создания требования указывают на то, в какие направления
	bool to_left_right [2];							// нужно отправить сообщения.
	to_left_right[0] = (INFO->neighbours_info[cpref_left][0].num_id != "***") && (INFO->neighbours_info[cpref_left][0].num_id != "~~~");
	to_left_right[1] = (INFO->neighbours_info[cpref_right][1].num_id != "***") &&(INFO->neighbours_info[cpref_right][1].num_id != "~~~");
	if (to_left_right[0]) {
		left_msg.kid = kid_counter++;
		left_msg.y = q;
		left_msg.q = announce_peer;
		left_msg.a.announced_node = INFO->self_info;
				// TODO на потом: требуется прикладывать к этому запросу подписанный proof_of_search
		left_msg.a.start_node = nif->self_info;	// Инициатор запроса - мы, как пир.
		left_msg.a.lvl = 0;        	              // "Добавь меня соседом начиная с 0-го уровня".
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
				// TODO на потом: требуется прикладывать к этому запросу подписанный proof_of_search
		right_msg.a.start_node = nif->self_info;	// Инициатор запроса - мы, как пир.
		right_msg.a.lvl = 0;
		right_msg.sender = INFO->self_info;
		right_msg.recipient = INFO->neighbours_info[cpref_right][1];
		right_msg.isanswered = false;

		right_msg.SendKRPCmsg(Client_);
	}

	request temprq;							// Создаём требование, которое потом передаст управление функции init3.
	temprq.name = "init3";
	temprq.count = to_left_right[0] && to_left_right[1] ?  2 : (to_left_right[0] || to_left_right[1] ? 1 : 0);
			// В требовании 0, 1 или 2 запроса в зависимости от того, сколько сообщений отправляли.
	temprq.q.reserve(temprq.count);
	temprq.r.reserve(temprq.count);			// Выделяем память.
	if (to_left_right[0])                   // Запихиваем в требование запросы.
			temprq.q.push_back(left_msg);   // Первым всегда идёт левый.
	if (to_left_right[1])
			temprq.q.push_back(right_msg);
	temprq.answered_count = 0;				// Пока ответов не пришло.

	req.push_back(temprq);
}

void init3 (request R, TIdUDPClient* Client_)
{
	strlist NIS = new TStringList;				// Сначала ищем адресный файл для вставляемого узла. num_id был записан в info_hash запроса на поиск.
	node_type typenode = peer;	// Является ли вставляемый узел пиром?
	if (R.q[0].a.info_hash != nif->self_info.num_id)   							// Если num_id вставляемого узла не совпадает с нашим,
		if (FileExists(GetCurrentDir() + "\\tx\\" + R.q[0].a.announced_node.num_id + ".txt")){      // надо найти и открыть файл
			NIS->LoadFromFile(GetCurrentDir() + "\\tx\\" + R.q[0].a.announced_node.num_id + ".txt"); // с данными соответствующего
			typenode = tx;                                                             // блока или транзакции.
			}
		else if (FileExists(GetCurrentDir() + "\\blk\\" + R.q[0].a.announced_node.num_id + ".txt")){
			NIS->LoadFromFile(GetCurrentDir() + "\\blk\\" + R.q[0].a.announced_node.num_id + ".txt");
			typenode = blk;
			}
		else
		{
			ShowMessage("Required ID not found in stored blocks and transactions!");
			return;
		}                                                                       // В случае совпадения используем данные файла nif.
																				// Действуем от имени вставляемого узла.
	node_info* INFO;                                                            // Создаём переменную с информацией о соседях
	INFO = (typenode == peer) ? nif : (new node_info(NIS));							  // на всех уровнях.
																				// Один или 2 ответа пришло?
	if (R.count == 1)     // Если нам приходит один ответ, это означает, что с другой стороны можно смело писать край.
	{   int cpref_former = common_prefix (INFO->self_info.name_id, R.q[0].recipient.name_id);	// Общий префикс со старым соседом.
		int cpref_new 	 = common_prefix (INFO->self_info.name_id, R.r[0].r.values.name_id);    // Общий префикс с новым.
						  // Если и с этой стороны новый сосед - край, то это конец.
		if (cpref_new == -1)
			return;
		else {            // Слева или справа пришёл ответ?
			compare_result neighbour_pos = compare_addr(INFO->self_info.num_id, R.r[0].r.values.num_id);
			for (int i = cpref_former + 1; i <= cpref_new; i++)
			{
			INFO->neighbours_info[i][neighbour_pos] = R.r[0].r.values;   // с той стороны, откуда пришло, пишем нормального соседа.
			INFO->neighbours_info[i][1 - neighbour_pos] = (neighbour_pos == left ? RIGHT_EDGE : LEFT_EDGE);
			}             // С другой стороны пишем край.
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
		compare_result neighbour_pos [2] = { compare_addr(INFO->self_info.num_id, R.r[0].r.values.num_id),	// Слева или справа пришёл
											 compare_addr(INFO->self_info.num_id, R.r[1].r.values.num_id) };	// каждый из ответов?

		int cpref_former [2] = {	common_prefix(INFO->self_info.name_id, R.q[0].recipient.name_id),
									common_prefix(INFO->self_info.name_id, R.q[1].recipient.name_id) };

		int cpref_new [2] 	 = {    common_prefix(INFO->self_info.name_id, R.r[0].r.values.name_id),
									common_prefix(INFO->self_info.name_id, R.r[1].r.values.name_id) };

		int former_size = INFO->neighbours_info.size();
		INFO->neighbours_info.resize((cpref_new[0] > cpref_new[1] ? cpref_new[0] : cpref_new[1]) + 1);
		for (unsigned int  i = former_size; i < INFO->neighbours_info.size(); i++)
			INFO->neighbours_info[i].resize(2);
		// Добавляем столько уровней, сколько максимальная длина общих префиксов с новыми соседями, и на каждом из новых уровней
		// создаём пару под двух соседей.
		if (R.r[0].r.values.num_id == "***" || R.r[0].r.values.num_id == "~~~")
			if (R.r[1].r.values.num_id == "***" || R.r[1].r.values.num_id == "~~~") // Слева край, справа край.
				return;
			else {
				for (int i = cpref_former[1] + 1; i < cpref_new[1]; i++)			// Слева край, справа не край.
					 INFO->neighbours_info[i][1] = R.r[1].r.values;             // Пишем нормального соседа справа.
				for (int i = cpref_former[0] + 1; i < cpref_new[1]; i++)        // Пишем край слева до общего префикса с ПРАВЫМ соседом.
					 INFO->neighbours_info[i][0] = LEFT_EDGE;
				}

		else if (R.r[1].r.values.num_id == "***" || R.r[1].r.values.num_id == "~~~"){  	// Слева не край, справа край.
				for (int i = cpref_former[0] + 1; i < cpref_new[0]; i++)         // Пишем нормального соседа слева.
					INFO->neighbours_info[i][0] = R.r[0].r.values;              // Пишем край справа до общего префикса с ЛЕВЫМ соседом.
				for (int i = cpref_former[1] + 1; i < cpref_new[0]; i++)
					INFO->neighbours_info[i][1] = RIGHT_EDGE;
			}
			 else {                          	                                // Слева не край, справа не край.
				for (int i = cpref_former[0] + 1; i < cpref_new[0]; i++)         // Пишем нормальных соседей справа и слева.
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

		temprq.r.reserve(responses_count);										// Выделяем место под столько ответов, сколько
		temprq.count = responses_count;											// отправили запросов.

	   	temprq.answered_count = 0;
		req.push_back(temprq);
	}
}
//---------------------------------------------------------------------------

void process_request(request R, TIdUDPClient* Client_)    // обработчик, вызывающийся при получении ответа на все запросы требования. В каждом
{								   // case нужно добавить исключение требования из вектора требований после выполнения функции.
	if (R.name == "init2")
			init2(R, Client_); // initx принимают на вход требование.

	if (R.name == "init3")
			init3(R,Client_);

};
