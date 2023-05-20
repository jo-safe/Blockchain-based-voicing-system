#pragma once


UnicodeString make_ip(UnicodeString ip);
enum compare_result { left, right, equally };
enum type_query { find_node, ping, get_peers, announce_peer, ask_ip };
enum CMD { q, r, e };
enum node_type { tx, blk, peer };

strlist cut_str(UnicodeString str) {
	strlist str_arr = new TStringList();
	UnicodeString element = "";
	int i = 1, is_it_node = 0;
	while(1) {
		if (is_it_node == 0) {
			if (i > str.Length()) break;
			while(1) {							// считываем слова, если не наткнулись на node.
				if (i > str.Length()) break;
				while(str[i] != ' ') {							// считываем слово до пробела
					if (i > str.Length()) break;
					element += str[i];
                    i++;
				}
				if (element != "!") {                // если считали не !, кладём то что считали.
					if (i > str.Length()) break;
					str_arr->Add(element);
					element = "";
				}
				i++;						// пропускаем пробел
                if (element == "!") {
					is_it_node = 1;
					break;
                }
			}
		}
		if (is_it_node == 1) {                 // если мы считали узел,
			if (i > str.Length()) break;
			element = "!";
			while(str[i] != '!') {            // пишем данные об узле в строку и добавляем её, начиная с !.
					element += str[i];
					i++;
			}
			str_arr->Add(element);
			is_it_node = 0;
		i = i + 2;								// пропускаем ! и ' '
		element = "";
		}
	}
	return str_arr;
}

struct node
{
	UnicodeString num_id;
	UnicodeString name_id;
	strlist ip;

	node() {
		num_id = name_id = "NI";
		ip = new TStringList();
		ip->Add("NI");
	}

	node(UnicodeString str){                                    // конструктор создания из строки с !
		strlist cstr = cut_str(str);							// ! попадёт в num_id
		num_id  = cstr->Strings[0].SubString(2,cstr->Strings[0].Length() - 1);		// отрезаем его.
		name_id = cstr->Strings[1];
		ip = new TStringList();
		for (int i = 0; i < cstr->Count-2; i++)
			ip->Add(cstr->Strings[i+2]);
	}

	void print() {
		cout << "num_id =" << num_id.c_str() << endl;
		cout << "name_id =" << name_id.c_str() << endl;
		for (int i = 0; i < ip->Count; i++) cout << ip->Strings[i].c_str() << endl;
	}

	node(const node& n) {
		num_id = n.num_id;
		name_id = n.name_id;
		ip = new TStringList();
		for (int i = 0; i < n.ip->Count; i++)
			ip->Add(n.ip->Strings[i]);
	}

	node& operator=(const node& N){		// Вызов вида a.=(N) .
		num_id = N.num_id;
		name_id = N.name_id;
		ip = new TStringList();

		for (int i = 0; i < N.ip->Count; i++)
			ip->Add(N.ip->Strings[i]);
		return *this;
	}
   //	bool is_nonpeer;   // 0 - указан 1 ip адрес, 1 - указано IP_COUNT ip адрессов
};

extern node RIGHT_EDGE;
extern node LEFT_EDGE;

node ReadNodeFromString(UnicodeString str, int* numsymbols){	// Возвращает node и кладёт в numsymbols количество реально
	node result;                           // кодирующих узел символов из строки ФАЙЛОВОГО ФОРМАТА.

	if (str[1] == '*'){
		*numsymbols = 9;
		result = RIGHT_EDGE;
		return result;
	}
	if (str[1] == '~'){
		*numsymbols = 9;
		result = LEFT_EDGE;
		return result;
	}
	result.num_id  = str.SubString(1, 20);
	result.name_id = str.SubString(21, 20);
	if (str[41] == '#')							  // Решётка - признак того что ждать 1 адрес.
		{
			result.ip->Strings[0] = str.SubString(42, 10);    // если у нас указан только один ip адрес
			*numsymbols = 51;			// был считан 51 символ.
			//self_info.is_nonpeer = 0;
		}
	else
		{
			//self_info.is_nonpeer = 0;
		for (int i = 0; i < IP_COUNT; i++)
			{                        // если указано IP_COUNT ip адресов
			result.ip->Add(str.SubString(42 + i * 10, 10));
			*numsymbols = 40 + 10 * IP_COUNT;			// Были считаны столько символов, т.к. нет диеза.
			}
		}
	return result;
}

UnicodeString ReadStringFromNode(node N){
	UnicodeString result = "";
	result += N.num_id;
	result += N.name_id;

	if (N.num_id == N.name_id)
		result += ("#" + N.ip->Strings[0]);
	else for (int i = 0; i < N.ip->Count; i++) {
			 result += N.ip->Strings[i];
		 }
	return result;
}
// Формат записей node_info в строку:
// <20 символов num-id или 3 символа края><20 символов name id или 3 символа края><От одного до IP_COUNT десятков символов IP или 3 символа края>.
class node_info
{   public:
	vector< vector<node> > neighbours_info;
	node self_info;                        // информация об узле
  //	node neighbours_info[160][2];		   	// информация о правом и левом соседях узла на каждом из уровней
											// [0] - левый сосед [1] - правый
	node_info(strlist S)
	{   neighbours_info.resize(S->Count - 1);	// Резервируем память под Count - 1 уровней, т.к. первая строка кодирует не соседей,.
		for (int i = 0; i < S->Count - 1; i++)		// а сам узел.
			neighbours_info[i].resize(2);   // На каждом из уровней резервируем память под двух соседей.

		int symbol_counter = 0;                                                 // Считываем данные о самом узле.
		self_info = ReadNodeFromString(S->Strings[0], &symbol_counter);

		for (int i = 1; i < S->Count && (left || right); i++)
		{   neighbours_info[i-1][0] = ReadNodeFromString(S->Strings[i], &symbol_counter);      // Считываем каждого левого соседа.
			neighbours_info[i-1][1] = ReadNodeFromString(S->Strings[i].SubString(symbol_counter + 1, S->Strings[i].Length() - symbol_counter), &symbol_counter);
																							// Считываем каждого правого соседа.
		}
	}

	strlist ToStringList()
	{
		strlist list = new TStringList();

		list->Add(ReadStringFromNode(self_info));
		for (unsigned int i = 1; i < neighbours_info.size() + 1; i++)
			list->Add(ReadStringFromNode(neighbours_info[i-1][0]) + ReadStringFromNode(neighbours_info[i-1][1]));

		return list;
	}
};

struct arguments
{
	UnicodeString target; 			// адрес цели поиска по name_id (LARAS)
	UnicodeString info_hash;        // адрес цели поиска по num_id (стандартный алгоритм).
   //	bool implied_port; 				// If it is non-zero, the port argument should be ignored and the source port
									// of the UDP packet should be used as the peer's port instead
	node announced_node;			// информация о новом узле для announce_peer
	int port;
	UnicodeString token; 			// proof-of-search
	node start_node; 				// адреса и ip инициатора поиска, вставки, или иного алгоритма. Это всегда пир.
	int lvl; 						// уровень скип графа при пересылке этого запроса
	//bool type_search; 			// 0 - поиск по num_id, 1 - поиск по nameID (адрес предыдущего блока в случае не-пира)
	bool is_validated; 				// была ли транзакция проверена всеми валидаторами?
};

struct responses
{
	vector<node> nodes;            	// это поле содержит адрес или адреса, наиболее близкие к тем, которые ищут
	int lvl;						// nodes = <NumIDLeft><NameIDLeft><IPLeft><NumIDRight><NameIDRight><IPRight>
	node values;           			// это поле содержит конкретный адрес или адреса, удовлетворяющие условиям запроса,
};                                  // если они найдены. В случае инициализации нового пира это поле содержит выдаваемый
									// ему num_id, а также IP, под которым он виден из Интернета: <NumID><NameID><IP>.
							// Также в случае если алгоритм дошёл до края скип графа, это поле содержит ~~~ = -inf или *** = +inf.
struct errors
{
	int e_code;
	UnicodeString e_name;

	errors(){
	}

	errors(UnicodeString str1, UnicodeString str2){
		if (str1=="") {
            e_code=0;
            e_name="";
        }
        else {
			e_code=StrToInt(str1);
			e_name=str2;
		}
	}
};

struct KRPCmsg
{
	int kid; 	        // id сообщения

	CMD y; 				        // тип сообщения: запрос, ответ, или ошибка
	type_query q;               // тип запроса. При отправке запроса переменная переписывается в требование.
	arguments a;                // данные запроса
	responses r;                // данные ответа
	errors e;                   // данные ошибки
	UnicodeString v;        		        // версия протокола
	UnicodeString proof_of_search;	// это поле содержит подписи всех участников цепочки поиска

	node sender;				// адреса отправителя сообщения (возможно, не-пир)
	node recipient;				// адреса получателя (возможно, не-пир)

	bool isanswered; 			// получен ли ответ на запрос?

	KRPCmsg();
	KRPCmsg(const KRPCmsg& M);
	void SendKRPCmsg(TIdUDPClient* Client_);
	KRPCmsg& operator= (const KRPCmsg& M);

};
KRPCmsg::KRPCmsg(const KRPCmsg& M){
	kid = M.kid;
	y = M.y;
	q = M.q;
	a.target = M.a.target;
	a.info_hash = M.a.info_hash;
	a.announced_node = M.a.announced_node;
	a.port = M.a.port;
	a.token = M.a.token;
	a.start_node = M.a.start_node;
	a.lvl = M.a.lvl;
	a.is_validated = M.a.is_validated;
	v = M.v;
	proof_of_search = "NI";

	for (unsigned int i = 0; i <  M.r.nodes.size(); i++)
		r.nodes.push_back(M.r.nodes[i]);

	r.lvl = M.r.lvl;
	r.values = M.r.values;
	e.e_code = M.e.e_code;
	e.e_name = M.e.e_name;
	sender = M.sender;
	recipient = M.recipient;
	isanswered = M.isanswered;
}

KRPCmsg& KRPCmsg::operator=(const KRPCmsg& M){
	kid = M.kid;
	y = M.y;
	q = M.q;
	a.target = M.a.target;
	a.info_hash = M.a.info_hash;
	a.announced_node = M.a.announced_node;
	a.port = M.a.port;
	a.token = M.a.token;
	a.start_node = M.a.start_node;
	a.lvl = M.a.lvl;
	a.is_validated = M.a.is_validated;

	for (unsigned int i = 0; i <  M.r.nodes.size(); i++)
		r.nodes.push_back(M.r.nodes[i]);

	r.lvl = M.r.lvl;
	v = M.v;
	r.values = M.r.values;
	e.e_code = M.e.e_code;
	e.e_name = M.e.e_name;
	sender = M.sender;
	recipient = M.recipient;
	isanswered = M.isanswered;

	return *this;
}

KRPCmsg::KRPCmsg (){
		node non_initialized_node;
		non_initialized_node.num_id = non_initialized_node.name_id = "NI";
		non_initialized_node.ip =new TStringList();
		non_initialized_node.ip->Add("NI");               // В KRPCMsg ВСЕГДА есть хотя бы один ip во всех нодах.
		kid = 0;
		y = CMD::q;
		q = type_query::ping;
		a.target = a.info_hash = a.token = proof_of_search = "NI";
		a.announced_node = non_initialized_node;
		r.lvl = a.port = a.lvl = -1;
		isanswered = a.is_validated = false;
		r.nodes.push_back(non_initialized_node);
		recipient = sender = r.values = non_initialized_node;   // *LAJA* из за этого возникает ошибка при обработке полученного запроса. Нужно чтобы recipient имел значение даже для первого отправляемого сообщения
		e.e_code = -1;
		e.e_name = "NI";
		v = '0';
	}

struct request
{
	UnicodeString name;			// какой функции передавать управление?
	vector <KRPCmsg> r;                 // массив ответов
	vector <KRPCmsg> q;					// массив запросов
	int count; 					// количество запросов, на который нужно получить ответ для выполнения требования
	int answered_count;			// количество запросов, на который уже пришли ответы
};

struct tx
{
	UnicodeString prev;                         // хеш предыдущего блока, он же name_id транзакции
	UnicodeString owner;						// num_id пира-владельца транзакции
	UnicodeString contents;
	UnicodeString search_proof;
	UnicodeString h;                            // хеш от (prev + owner + contents + search_proof), он же num_id транзакции
	UnicodeString sigma;
};

enum CMD str_to_CMD (const UnicodeString& str){
    if (str=="q") return q;
    if (str=="r") return r;
	else return e;
}

enum type_query str_to_type_query (const UnicodeString& str){
    if (str=="find_node") return find_node;
    if (str=="ping") return ping;
    if (str=="get_peers") return get_peers;
    if (str=="announce_peer") return announce_peer;
	if (str=="ask_ip") return ask_ip;
	else return ping;
}

//из строчки делаем булку
bool str_to_bool (const UnicodeString& str){
    if (str=="0") return false;
	if (str=="1") return true;
	else {ShowMessage("StrToBool error."); return false;};
}

UnicodeString node_to_str(node Node){   		// делает из NOde строку СЕТЕВОГО ФОРМАТА.
	UnicodeString ret="! ";
	ret=ret+Node.num_id+" "+Node.name_id+" ";
	for (int i = 0; i < Node.ip->Count; i++) ret=ret+Node.ip->Strings[i]+" ";
	ret+="! ";
	return ret;
}

KRPCmsg BDecode(UnicodeString str){
	KRPCmsg ret;
	strlist cstr=cut_str(str);

	ret.kid=StrToInt(cstr->Strings[0]);
	ret.y=str_to_CMD(cstr->Strings[1]);
	ret.q=str_to_type_query(cstr->Strings[2]);

	ret.a.target=cstr->Strings[3];
	ret.a.info_hash=cstr->Strings[4];
	ret.a.port=StrToInt(cstr->Strings[5]);
	ret.a.token=cstr->Strings[6];
	ret.a.lvl=StrToInt(cstr->Strings[7]);
	ret.a.is_validated=str_to_bool(cstr->Strings[8]);

	node a_n(cstr->Strings[9]);
	ret.a.announced_node=a_n;

	node s_n(cstr->Strings[10]);
	ret.a.start_node=s_n;

	int k=11;

	while(1){                          			// читаем nodes
		if (cstr->Strings[k][1]=='!') {
//				node Node(cstr->Strings[k]);
				ret.r.nodes.push_back(node(cstr->Strings[k]));
				k++;
        }
		else break;
    }

	ret.r.lvl=StrToInt(cstr->Strings[k]);
	node va(cstr->Strings[k+1]);
	ret.r.values=va;

	errors er(cstr->Strings[k+2],cstr->Strings[k+3]);
	ret.e=er;

	ret.v=cstr->Strings[k+4][1];
	ret.proof_of_search=cstr->Strings[k+5];
	node sender(cstr->Strings[k+6]);
	node recipient(cstr->Strings[k+7]);
	ret.sender=sender;
	ret.recipient=recipient;
	ret.isanswered=str_to_bool(cstr->Strings[k+8]);
	/*int test=1;
    if (test==1) {
		cout << "kid=" << ret.kid << endl;
        cout << "y=" << ret.y << endl;
        cout << "q=" << ret.q << endl;
		cout << "a.target=" << ret.a.target << endl;
        cout << "a.info_hash=" << ret.a.info_hash << endl;
        cout << "ret.a.port=" << ret.a.port << endl;
        cout << "ret.a.token=" << ret.a.token << endl;
        cout << "ret.a.lvl=" << ret.a.lvl << endl;
        cout << "ret.a.is_validated=" << ret.a.is_validated << endl;
        cout << "announce_node is:" << endl;
        ret.a.announced_node.print();
        cout << "start_node is:" << endl;
		ret.a.start_node.print();
        cout << "responses nodes are:" << endl;
		for (int i=0; i < ret.r.nodes.size(); i++) {
                ret.r.nodes[i].print();
        }
        cout << "lvl=" << ret.r.lvl << endl;
        cout << "values node is " << endl;
        ret.r.values.print();
        cout << "error_code=" << ret.e.e_code << endl;
        cout << "error_name=" << ret.e.e_name << endl;
        cout << "version of protocol=" << ret.v << endl;
        cout << "proof_of_search=" << ret.proof_of_search << endl;
        cout << "sender is:" << endl;
		ret.sender.print();
        cout << "recipient is:" << endl;
        ret.recipient.print();
        cout << "is_answered is:" << ret.isanswered << endl;
	}           */
    return ret;
}

UnicodeString BEncode(KRPCmsg el){
    UnicodeString ret="";
    char buffer[120];
	ret+=IntToStr(el.kid)+" ";
	if (el.y==0) ret+="q ";
	if (el.y==1) ret+="r ";
	if (el.y==2) ret+="e ";
	if (el.q==0) ret+="find_node ";
	if (el.q==1) ret+="ping ";
	if (el.q==2) ret+="get_peers ";
	if (el.q==3) ret+="announce_peer ";
	if (el.q==4) ret+="ask_ip ";

	ret=ret+el.a.target+" "+el.a.info_hash+" ";
	ret=ret+IntToStr(el.a.port)+" "+el.a.token+" "+IntToStr(el.a.lvl)+" "+BoolToStr(el.a.is_validated)+" ";
	ret=ret+node_to_str(el.a.announced_node)+node_to_str(el.a.start_node);

	for (unsigned int i=0; i < el.r.nodes.size(); i++)
		ret+=node_to_str(el.r.nodes[i]);
	ret += IntToStr(el.r.lvl) + " " + node_to_str(el.r.values);
	if (el.e.e_code==-1) ret+="  "; 		// два пробела
	else ret=ret+IntToStr(el.e.e_code)+" "+el.e.e_name+" ";
	UnicodeString version = el.v;
	ret+=version+" ";
	ret+=el.proof_of_search+" ";
	ret+=node_to_str(el.sender)+node_to_str(el.recipient);
	ret=ret+BoolToStr(el.isanswered)+" ";
	ShowMessage(ret);
	return ret;
}

void KRPCmsg::SendKRPCmsg(TIdUDPClient* Client_){
	if (recipient.ip->Strings[0] == "***" || recipient.ip->Strings[0] == "~~~") return;
	else{
		Client_->Host = make_ip(recipient.ip->Strings[0]);		// Выставляем сокет на отправку данных инициатору поиска,

		Client_->Active = true; 				// выключаем сервер в целях тестирования, чтобы не занимать сокет.
		Client_->Send(BEncode(*this)); 			// Он включается по нажатию кнопки.
		Client_->Active = false;
	}
}
