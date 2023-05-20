# Blockchain-based-voicing-system
Voicing system based on DHT blockchain LightChain
Using RAD Studio XE7, so data-types like UnicodeString are the same as wstring from STD, TStringList is like vector<string> or something. So the point is the same, the methods are a bit different.
  
Project has four modules:
  - KRPCMSG.h contents methods to generate messages between peers 
  - queries.h operates queries 
  - responces.h - likewise
  - secfunctions.h includes secondary methods which STL library with RAD Studio XE7 didn't content when the project had been written
The main conceptions used in project:
  - blockchain protocol Proof of Validation
  - DHT decentralized protocol to exchange data between peers
  - SHA-1 with 160-bit digest
  - Skip-graph technology to operate connection between peers on different levels (realized graph can content 2^36 peers)
  
Unfortunately, these are almost all files that I have from that distant times when I was writing this project. Now I am portering it to windows CLR with web system based on win32 API.
