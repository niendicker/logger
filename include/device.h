#ifndef DEVICE_H
#define DEVICE_H

#include <list>
#include <string>

using namespace std;

class dataSource
{
private:
  int protocol;
  int timeout;
  string* dataSourceID;
public:
  dataSource(/* args */);
  virtual ~dataSource();
  virtual int connect() = 0;
  virtual int disconnect() = 0;
  virtual int readData() = 0;
};

dataSource::dataSource(/* args */)
{
  this->dataSourceID = new string("default");
}

dataSource::~dataSource()
{
  delete(this->dataSourceID);
}

class mbTcpDevice : public dataSource
{
private:
  /* data */
  string* hostname;
  string* ipv4;
  int port;
public:
  mbTcpDevice(/* args */){};
  ~mbTcpDevice(){};
  int connect(){ return 0; };
  int disconnect(){ return 0; };
  int readData(){ return 0; };
};


#endif