#pragma once

/*UnicodeString make_ip(UnicodeString);                  //������� ip �� ������� 2130706433 � 127.0.0.1
int make_num_ip(UnicodeString);                        //������� ip �� ������� 127.0.0.1 � 2130706433(int)
UnicodeString make_id (int);                           //�������� ������ id
uint_least32_t crc32c_wiki(UnicodeString, size_t);     //...
compare_result compare_addr(UnicodeString, UnicodeString);      //��������� id �����
int common_prefix(UnicodeString left, UnicodeString right)		// ����� ������ �������� name_id
UnicodeString IdToString(UnicodeString);               //...
void make_ip_10(UnicodeString&);                       //���� ip(string) ������ 10 ��������, ����������� ���� �����
void sdvig(UnicodeString&, int, int);                  //
void GenDSAKeyPair();    							   // ������������� ���� ������ - ��������� ��� ������ ��������!
void ReadDSAKeyPair();								   // ������� ���� ������ �� �����
*/
void sdvig(UnicodeString& arr, int i, int n)
{
	UnicodeString* a = &arr;
	UnicodeString temp[2];
	temp[(i + 1) % 2] = a[i];
	for (int m = i; m < n; m++)
	{
		temp[m % 2] = a[m + 1];
		a[m + 1] = temp[(m + 1) % 2];
	}
}
       /*

uint_least32_t crc32c_wiki(UnicodeString buf, size_t len)
{
	uint_least32_t crc_table[256];
    uint_least32_t crc; int i, j;

	for (i = 0; i < 256; i++)
	{
        crc = i;
        for (j = 0; j < 8; j++)
			crc = crc & 1 ? (crc >> 1) ^ 0x82F63B78 : crc >> 1;

        crc_table[i] = crc;
    };

    crc = 0xFFFFFFFFUL;
    i = 1;

	while (len--)  {
		crc = crc_table[(crc ^ buf[i]) & 0xFF] ^ (crc >> 8);
		i++;
		}

    return crc ^ 0xFFFFFFFFUL;
}



UnicodeString make_id (int ip_input)
{

	uint8_t* ip; // our external IPv4 or IPv6 address (network byte order)
	int num_octets = 4; // the number of octets to consider in ip (4 or 8)
	ip = new uint8_t [num_octets];

	ip[0] = ip_input % 256;
	ip[1] = ip_input / 256 % 256;
	ip[2] = ip_input / 256 / 256 % 256;
	ip[3] = ip_input / 256 / 256 / 256 % 256;

	uint8_t node_id[20]; // resulting node ID

	uint8_t v4_mask[] = { 0x03, 0x0f, 0x3f, 0xff };
	uint8_t v6_mask[] = { 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff };
	uint8_t* mask = num_octets == 4 ? v4_mask : v6_mask;

	for (int i = 0; i < num_octets; ++i)
		ip[i] &= mask[i];

	uint32_t rand = std::rand() & 0xff;
	uint8_t r = rand & 0x7;
	ip[0] |= r << 5;

	uint32_t crc = 0;
	UnicodeString s = make_ip(ip_input);
	crc = crc32c_wiki(s, s.Length());

	// only take the top 21 bits from crc
	node_id[0] = (crc >> 24) & 0xff;
	node_id[1] = (crc >> 16) & 0xff;
	node_id[2] = ((crc >> 8) & 0xf8) | (std::rand() & 0x7);
	for (int i = 3; i < 19; ++i) node_id[i] = std::rand();
	node_id[19] = rand;

	UnicodeString s_output;

	for (int i = 0; i < 20; i++)
	{
     	s_output += IntToHex(node_id[i], 1);
	}
	return s_output;
}
            */


int make_num_ip(UnicodeString s)
{
	int a = 0;
	UnicodeString temp = "";
	int i = 1;

	s += ' ';
	for(; s[i] != '.'; i++)
	{
		temp += s[i];
	}
	i++;
	a += StrToInt(temp) * 256 * 256 * 256;
	temp = "";

	for(; s[i] != '.'; i++)
	{
		temp += s[i];
	}
	i++;
	a += StrToInt(temp) * 256 * 256;
	temp = "";

	for(; s[i] != '.'; i++)
	{
		temp += s[i];
	}
	i++;
	a += StrToInt(temp) * 256;
	temp = "";

	for(; s[i] != ' '; i++)
	{
		temp += s[i];
	}
	i++;
    a += StrToInt(temp);
	return a;
}



UnicodeString make_ip(UnicodeString ip)
{
	UnicodeString s;
	s += IntToStr(StrToInt(ip) / 256 / 256 / 256);
	s += '.';
	s += IntToStr(StrToInt(ip) / 256 / 256 % 256);
	s += '.';
	s += IntToStr(StrToInt(ip) / 256 % 256);
	s += '.';
	s += IntToStr(StrToInt(ip) % 256);
	return s;
}


UnicodeString IdToString(UnicodeString s)    // ������� id �� ��������� ������� � ��������
{
	AnsiString ansiB(s);					 // ��� ����� 1-�������� ������


	UnicodeString res = "";
	for(int i = 1; i <= 20; i++)
	{
	for(int j = 0; j < 8; j++)
		res += bitset<8>(s[i]).to_string()[j];
	}
	return res;
}

compare_result compare_addr(UnicodeString left, UnicodeString right)   // ��������� ������� num_id
{
	left = IdToString(left);
	right = IdToString(right);

	if (((left == "~~~") || (right == "~~~")) && left != right)
	{
		return left == "~~~" ? compare_result::right : compare_result::left;				// ����� ������������� ������
	}
	else if (((left == "***") || (right == "***")) && left != right)
	{
		return left == "***" ? compare_result::left : compare_result::right;				// ���� ������������� ������
	}
	else if (left == right) return equally;
	else for(int i = 1; i <= 160; i++) // ���������� ����������
	{
		if (StrToInt(left[i]) > StrToInt(right[i]))
		{
			return compare_result::left;         //���� ������ ID ������ �������
		}
		else
		if (StrToInt(left[i]) < StrToInt(right[i]))
		{
			return compare_result::right;          //���� ������ ID ������ �������
		}
	}
	return equally;
}


int common_prefix(UnicodeString left, UnicodeString right)
{                                             // ��������� ���������� name_id
	left = IdToString(left);
	right = IdToString(right);

	if (((left == "~~~") || (right == "~~~")) && left != right)
	{
		return -1;
	}
	else if (((left == "***") || (right == "***")) && left != right)
	{
		return -1;
	}
	else if (left == right) return equally;
	else for(int i = 1; i <= 160; i++) // ���������� ����������
	{
		if (StrToInt(left[i]) != StrToInt(right[i]))
			return i;
	}
	return 160;
}


void make_ip_10(UnicodeString& ip)
{
	for(int i = 1; ip.Length() != 10; i++)
	{
		sdvig(ip, 1, 25);
		ip[i] = '0';
	}
}
			 /*
void GenDSAKeyPair()            		// COPYRIGHT TO jww
{
    int rc = 0;
    unsigned long err = 0;

   //	DSA_ptr key(DSA_new(), ::DSA_free);
	err = ERR_get_error();

	unsigned char buffer[32];
    rc = RAND_bytes(buffer, sizeof(buffer));

    if (rc != 1)
		ShowMessage("RAND_bytes failed, error " + IntToStr(err) + ", " + "0x" + HexToStr(err) );

	rc = DSA_generate_parameters_ex(SelfDSAKey.get(), 2048 , buffer, sizeof(buffer), NULL, NULL,
            NULL);
    err = ERR_get_error();

	if (rc != 1)
		ShowMessage("DSA_generate_parameters_ex failed, error " + IntToStr(err) + ", " + "0x" + HexToStr(err) );

	rc = DSA_generate_key(SelfDSAKey.get());
    err = ERR_get_error();

	if (rc != 1)
		ShowMessage("DSA_generate_key failed, error " + IntToStr(err) + ", " + "0x" + HexToStr(err) );

	//EVP_PKEY_ptr pkey(EVP_PKEY_new(), ::EVP_PKEY_free);


	rc = EVP_PKEY_set1_DSA(SelfEVP.get(), SelfDSAKey.get());
	err = ERR_get_error();

	if (rc != 1)
		ShowMessage("EVP_PKEY_assign_DSA failed, error " + IntToStr(err) + ", " + "0x" + HexToStr(err) );

	BIO_MEM_ptr bio1(BIO_new(BIO_s_mem()), ::BIO_free);

	rc = PEM_write_bio_PKCS8PrivateKey(bio1.get(), SelfEVP.get(), NULL, NULL, 0, NULL, NULL);
	err = ERR_get_error();

	if (rc != 1)
		ShowMessage("PEM_write_bio_PKCS8PrivateKey, error " + IntToStr(err) + ", " + "0x" + HexToStr(err) );

	BUF_MEM *mem = NULL;       			// OpenSSL-������ ���������, �������������� ������ ������� �������� char
	BIO_get_mem_ptr(bio1.get(), &mem);	// �������� ������ �� ������ BIO � ���������� mem
	err = ERR_get_error();

	if (!mem || !mem->data || !mem->length)
		ShowMessage("BIO_get_mem_ptr failed, error " + IntToStr(err) + ", " + "0x" + HexToStr(err) );

	string pem(static_cast<const char*>(mem->data), static_cast<size_t>(mem->length));
	UnicodeString unicodepem = pem;
	ShowMessage(unicodepem);


	BIO_FILE_ptr bio2(BIO_new_file(kf, "w"), ::BIO_free);    // ������ ���� � ������ �� ��������� kf �� ������ � ������� ��� � BIO, �� �������
															 // ��������� bio2
	if (bio2 == NULL)
		ShowMessage("BIO_new_file, error ");


	rc = i2d_DSAPrivateKey_bio(bio2.get(), SelfDSAKey.get());// ����� ���� �� SelfDSAKey � BIO, �� ������� ��������� bio2
	err = ERR_get_error();

	if (rc != 1)
		ShowMessage("i2d_DSAPrivateKey_bio, error " + IntToStr(err) + ", " + "0x" + HexToStr(err) );

}

void ReadDSAKeyPair(){
	BIO_FILE_ptr fbio(BIO_new_file(kf, "w"), ::BIO_free);

	if (fbio == NULL) {
	   ShowMessage("BIO new file error, error ");
	}

	SelfDSAKey = DSA_ptr(d2i_DSAPrivateKey_bio(fbio.get(), NULL), ::DSA_free);

	if (SelfDSAKey == NULL) {
	   ShowMessage("DSA pointer reading error ");
	}

	if (EVP_PKEY_set1_DSA(SelfEVP.get(), SelfDSAKey.get()) != 1) {
		ShowMessage("EVP assignment error ");
	}
}

UnicodeString GenAddr(){								// ������� SHA1-��� ������ ��������� �����. ������� ���� ��� ��������.
	unsigned char* hash;
	hash = new unsigned char [SHA_DIGEST_LENGTH];				// �������� 20 ���� ��� �������� ����
																// �� ����� � EVP_PKEY_ptr SelfEVP
	BUF_MEM *PubDSA = BUF_MEM_new();                    		// � ���� ������ �� ����� ������
	BIO_MEM_ptr bio1(BIO_new(BIO_s_mem()), ::BIO_free);         // �������� ������, � ������� ������� �������� ���� �� ����������

	if (!(PEM_write_bio_PUBKEY(bio1.get(), SelfEVP.get())) {	// ��������� �������� ����
		ShowMessage("Reading key to memory error.");
	}

	//BIO_write(bio, "\0", 1);                  // ���� �����, ��������� �������� ����� ������
	BIO_get_mem_ptr(bio1.get(), &PubDSA);		// ��������� ���������� BIO � �������
												// ������ ����� ��������� �� �� ���
	SHA1(PubDSA->data, PubDSA->length, hash);
	BIO_set_close(bio1.get(), BIO_NOCLOSE);

	return (UnicodeString)hash;
}          */
