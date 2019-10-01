// **************************************************************************************
// *  Web Server (web_server.cc)
// * -- Accepts and processes HTTP 1.0 GET requests
// **************************************************************************************
#include "web_server.h"

//********************************
// * sigHandler
// * Gracefully handles CTRL^C
//  *****************************   
void sigHandler(int dummyInt) {
    std:: cout << std::endl;
    std::cout << "Handling socket close" << std::endl;
    exit(0);
}

// print out string
void printStringBuffer(char *c) {
    std::cout << "strlen(c): " << strlen(c) << std::endl;
    for(size_t i = 0; i < strlen(c); i++) {
        std::cout << c[i] << std::endl;
    }
}

int getDelimeterIndex(char *buffer, int startingIndex) {
    while(startingIndex < strlen(buffer) && buffer[startingIndex] != ' ') {
        startingIndex++;
    }
    return startingIndex;
}

/**
 * Used to find the end of the line in the request
 **/
int findCarriageReturn(char *buffer) {
    DEBUG << "inside of findCarriageReturn" << ENDL;
    int index = 0;
    int carriage_index = -1;
    while(index < strlen(buffer)) {
      if(buffer[index] == '\n' || buffer[index] == '\r') {
        DEBUG << "found a carriage return at buffer[index]: " << buffer[index] << ENDL;
        carriage_index = index;
        return carriage_index;
      }
      index++;
    }
    return carriage_index;
}


char *getStr(char *buffer, int start, int end) {
    char *temp;
    temp =(char*)malloc((end - start +1)*1);
    int count = 0;
    for(size_t i = start; i < end; i++) {
        temp[count] = buffer[i];
        std::cout << "temp[count]: " << temp[count] << std::endl;
        count++;
    }
    std::cout << "temp[] in getStr(): " << temp << std::endl;
    return (char*)temp;
}

bool isCharArrayEmpty(char *buffer) {


  for(int i = 0; i < strlen(buffer); i++) {
    if(buffer[i] != 0)
      return false;
  }
  return true;
}    
// **************************************************************************************
// * processConnection()
// * - Handles reading the line from the network and sending it back to the client.
// * - Returns 1 if the client sends "QUIT" command, 0 if the client sends "CLOSE".
// **************************************************************************************
int processConnection(int sockFd) {

  int quitProgram = 0;
  int keepGoing = 1;
  int buffer_size = 10;
  char carriage_buffer[1024];
  int request_index = -1;
  int carriage_index = 0;
  bzero(carriage_buffer, sizeof(carriage_buffer));
  bool read_buffer = true;
  while (keepGoing) {
    // catch CTRL^C
    signal(SIGINT, sigHandler);

    //
    // Call read() call to get a buffer/line from the client.
    // Hint - don't forget to zero out the buffer each time you use it.
    //
    char buffer[buffer_size];

    bzero(buffer, sizeof(buffer));
    DEBUG << "Receiving block of data" << ENDL;
    read(sockFd, buffer, buffer_size);

    buffer[buffer_size] = '\0';
    
    if(strlen(buffer) == 0) {
      DEBUG << "Buffer was empty, closing connection" << ENDL;
      return 0;
    }

    request_index = findCarriageReturn(buffer);
    if(request_index != -1) {
      DEBUG << "A carriage return was found" << ENDL;
      
      // place all of the buffer containing data from the request header into carriage_buffer
      for(size_t i = 0; i < request_index; i++){
        carriage_buffer[carriage_index + i] = buffer[i];
      }
      
      // add the offset of the carriage_index 
      request_index += carriage_index;
      
      
      // find the GET in the request header
      int get_index = getDelimeterIndex(carriage_buffer, 0);

      // **************************************
      // check that GET keyword appears in request
      // **************************************
      char getMsg[get_index];
      
      for(int i = 0; i < get_index; i++){
        getMsg[i] = carriage_buffer[i];
      }

      // add end of string character
      getMsg[get_index] = '\0';

      // check if the request contains a correct GET message, if not send a 400 error
      if(strcmp(getMsg, "GET") != 0) {
        
        // write out a 400 error
        char *error = "HTTP/1.0 400 Bad Request\r\nContent-Length: 15\r\nConnection: close\r\n\r\n400 Bad Request";
        DEBUG << "Sending HTTP/1.0 400 Bad Request" << ENDL;
        write(sockFd, error, strlen(error));

        // keeps program going
        return 0;
      } else {
        DEBUG << "Was a valid GET request" << ENDL;
      }

      // find file name, index is + 1 what the get_index was
      int file_index = getDelimeterIndex(carriage_buffer, get_index + 1);

      // initialize space for the file name
      char fileMsg[file_index - get_index + 1];

      //initialize index for file msg since carriage buffer is starting at get_index + 1
      int count = 0;
      for(int i = get_index + 1; i < file_index; i++){
        fileMsg[count] = carriage_buffer[i];
        count++;
      }
      
      // Set the end of the string in fileMsg to get rid of empty characters in the buffer
      fileMsg[file_index - get_index - 1] = '\0';

      
      // check to see if fileMsg is a valid file name
      if(strcmp(fileMsg, "/file1.html") == 0 || strcmp(fileMsg, "/file2.html") == 0) { // check for html file
        DEBUG << "Was a valid html file name" << ENDL;

        // read file and build response
        FILE *fp;
        char ch;
        std::string fileName = std::string(fileMsg).substr(1);
        fp = fopen(fileName.c_str(), "r");

        if(fp == NULL) {
          DEBUG << "Error while opening valid file" << ENDL;
          exit(-1);
        }

        int file_size = 0;
        DEBUG << "File content:" << ENDL;
        
        fseek(fp, 0L, SEEK_END);
        file_size = ftell(fp);

        char body_msg[file_size];

        // rewind file pointer
        rewind(fp);
        int file_i = 0;
        while((ch = fgetc(fp)) != EOF) {
          body_msg[file_i] = ch;

          std::cout << body_msg[file_i];
          file_i++;
        }
        std::cout << std::endl;
        
        DEBUG << "Size of file: " << file_size << ENDL;
        DEBUG << "Size of body_msg: " << strlen(body_msg) << ENDL;
        // TODO: send successful response to GET request for file1.html file
        // TODO: insert correct content length
        char *successful_response = "HTTP/1.0 200 ok\r\nContent-Length: ";
        char content_size[8];

        DEBUG << "About to concatenate the size of the file" << ENDL;
        std::string resp = std::string(successful_response);

        resp.append(std::to_string(file_size));
        char *rest_of_response = "\r\nContent-type:text/html\r\n\r\n";
        resp.append(std::string(rest_of_response));

        resp.append(std::string(body_msg));
        DEBUG << "resp: " << resp << ENDL;

        write(sockFd, resp.c_str(), resp.size());

        fclose(fp);

        // keeps program going
        return 0;
      } else if(strcmp(fileMsg, "/image1.jpg") == 0 || strcmp(fileMsg, "/image2.jpg") == 0) { // check for image file
                DEBUG << "Was a valid html file name" << ENDL;
        // read file and build response
        FILE *fp;
        char ch;
        std::string fileName = std::string(fileMsg).substr(1);
        
        // read images in binary format
        fp = fopen(fileName.c_str(), "rb");

        if(fp == NULL) {
          DEBUG << "Error while opening valid file" << ENDL;
          exit(-1);
        }

        int file_size = 0;
        DEBUG << "File content:" << ENDL;
        

        // char body_msg[file_size];
        std::vector<char> body_msg;
        // rewind file pointer

        if(fp != nullptr) {
          fseek(fp, 0L, SEEK_END);
          file_size = ftell(fp);
          rewind(fp);
          body_msg.resize(file_size);
          // Content-Transfer-Encoding: binary;
          // fread(&body_msg[0], 1, file_size, fp);
          fread(&body_msg[0], 1, file_size, fp);
        }
        // int file_i = 0;
        // while((ch = fgetc(fp)) != EOF) {
        //   body_msg[file_i] = ch;

        //   std::cout << body_msg[file_i];
        //   file_i++;
        // }
        // std::cout << std::endl;
        
        DEBUG << "Size of file: " << file_size << ENDL;
        DEBUG << "Size of body_msg: " << body_msg.size() << ENDL;
        // TODO: send successful response to GET request for file1.html file
        // TODO: insert correct content length
        char *successful_response = "HTTP/1.0 200 ok\r\nContent-Length: ";
        char content_size[8];

        std::string resp = std::string(successful_response);

        resp.append(std::to_string(file_size));
        char *rest_of_response = "\r\nContent-type:image/jpeg; Content-Transfer-Encoding: binary;\r\n\r\n";
        resp.append(std::string(rest_of_response));

        for(char c: body_msg) {
          resp += c;
        }
        // DEBUG << "resp: " << resp << ENDL;

        write(sockFd, resp.c_str(), resp.size());

        fclose(fp);

        // keeps program going
        return 0;
      }
      else {
        
        // Print out 404 error
        // char *error = "HTTP/1.0 404 Not Found\n\n";
        char *error = "HTTP/1.0 404 Not Found\r\n\r\n404 Not Found";
        DEBUG << "Sending HTTP/1.0 404 Not Found" << ENDL;
        write(sockFd, error, strlen(error));
        
        // keeps program going
        return 0;
      }
    } else { // since a carriage return wasn't found, continue parsing through the socket data
      DEBUG << "Didn't find a carriage, continuing to read from socket" << ENDL;

      // build the carriage buffer since we still need to keep reading
      for(size_t i = 0; i < buffer_size; i++){
        carriage_buffer[carriage_index + i] = buffer[i];
      }
      
      //increment the carriage buffer since we added another 10 bytes
      carriage_index += buffer_size;
    }

  }

  return quitProgram;
}
    


// **************************************************************************************
// * main()
// * - Sets up the sockets and accepts new connection until processConnection() returns 1
// **************************************************************************************

int main (int argc, char *argv[]) {

  // ********************************************************************
  // * Process the command line arguments
  // ********************************************************************
  boost::log::add_console_log(std::cout, boost::log::keywords::format = "%Message%");
  boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);

  // ********************************************************************
  // * Process the command line arguments
  // ********************************************************************
  int opt = 0;
  while ((opt = getopt(argc,argv,"v")) != -1) {
    
    switch (opt) {
    case 'v':
      boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);
      break;
    case ':':
    case '?':
    default:
      std::cout << "useage: " << argv[0] << " -v" << std::endl;
      exit(-1);
    }
  }

  // *******************************************************************
  // * Creating the inital socket is the same as in a client.
  // ********************************************************************
  int     listenFd = -1;

  // Call socket() to create the socket you will use for listening.
  if((listenFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      std::cout << "Failed to create listening socket " << strerror(errno) << std::endl;
      exit(1);
    }

  DEBUG << "Calling Socket() assigned file descriptor " << listenFd << ENDL;
  
  // ********************************************************************
  // * The bind() and calls take a structure that specifies the
  // * address to be used for the connection. On the client it contains
  // * the address of the server to connect to. On the server it specifies
  // * which IP address and port to lisen for connections.
  // ********************************************************************
  struct sockaddr_in servaddr;
  srand(time(NULL));
  int port = (rand() % 10000) + 1024;
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = PF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);


  // ********************************************************************
  // * Binding configures the socket with the parameters we have
  // * specified in the servaddr structure.  This step is implicit in
  // * the connect() call, but must be explicitly listed for servers.
  // ********************************************************************

  DEBUG << "Calling bind(" << listenFd << "," << &servaddr << "," << sizeof(servaddr) << ")" << ENDL;
  int bindSuccessful = 0;
  while (!bindSuccessful) {
    // You may have to call bind multiple times if another process is already using the port
    // your program selects.
    if(bind(listenFd, (sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        std::cout << "bind failed: " << strerror(errno) << std::endl;
    } else {
        bindSuccessful = 1;
    }
  }
  std::cout << "Using port " << port << std::endl;


  // ********************************************************************
  // * Setting the socket to the listening state is the second step
  // * needed to being accepting connections.  This creates a queue for
  // * connections and starts the kernel listening for connections.
  // ********************************************************************
  int listenQueueLength = 1;
  DEBUG << "Calling listen(" << listenFd << "," << listenQueueLength << ")" << ENDL;
  if(listen(listenFd,listenQueueLength) < 0) {
      std::cout << "listen() failed: " << strerror(errno) << std::endl;
      exit(-1);
    }


  // ********************************************************************
  // * The accept call will sleep, waiting for a connection.  When 
  // * a connection request comes in the accept() call creates a NEW
  // * socket with a new fd that will be used for the communication.
  // ********************************************************************
  int quitProgram = 0;
  while (!quitProgram) {
    int connFd = 0;

    DEBUG << "Calling accept(" << listenFd << "NULL,NULL)." << ENDL;

    std::cout << std::endl;
  

    // The accept() call checks the listening queue for connection requests.
    // If a client has already tried to connect accept() will complete the
    // connection and return a file descriptor that you can read from and
    // write to. If there is no connection waiting accept() will block and
    // not return until there is a connection.
    if((connFd = accept(listenFd, (sockaddr *) NULL, NULL)) < 0) {
        std::cout << "accept() failed: " << strerror(errno) << std::endl;
        exit(-1);
    }
    DEBUG << "We have recieved a connection on " << connFd << ENDL;

    DEBUG << "Waiting for data from client" << ENDL;  
    quitProgram = processConnection(connFd);

    close(connFd);

    std::cout << std::endl;
    std:: cout << std::endl;
  }
  
  DEBUG << "after closing socket with client, closing the server socket" << ENDL;

  close(listenFd);

}
