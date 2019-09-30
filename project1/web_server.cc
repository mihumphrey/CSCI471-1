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
    std::cout << "inside getDelimeterIndex, startingIndex: " << startingIndex << std::endl;
    while(startingIndex < strlen(buffer) && buffer[startingIndex] != ' ') {
        startingIndex++;
    }
    return startingIndex;
}

/**
 * Used to find the end of the line in the request
 **/
int findCarriageReturn(char *buffer) {
    std::cout << "inside of findCarriageReturn" << std::endl;
    int index = 0;
    int carriage_index = -1;
    while(index < strlen(buffer)) {
      if(buffer[index] == '\n' || buffer[index] == '\r') {
        std::cout << "found a carriage return at buffer[index]: " << buffer[index] << std::endl;
        carriage_index = index;
        return carriage_index;
      }
      index++;
    }
    std::cout << "carriage_index before returning" << carriage_index << std::endl;
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

  while (keepGoing) {
    std::cout << "iterating in keepGoing loop" << std::endl;
    signal(SIGINT, sigHandler);
    //
    // Call read() call to get a buffer/line from the client.
    // Hint - don't forget to zero out the buffer each time you use it.
    //
    char buffer[buffer_size];

    bzero(buffer, sizeof(buffer));
    DEBUG << "Receiving block of data" << ENDL;
    read(sockFd, buffer, buffer_size);

    DEBUG << "buffer: " << buffer << ENDL;
    request_index = findCarriageReturn(buffer);
    if(request_index != -1) {
      std::cout << "We found a carriage whoo!!, request_index: " << request_index << std::endl;
      // finish building the string
      for(size_t i = 0; i < request_index; i++){
        carriage_buffer[carriage_index + i] = buffer[i];
      }
      // parse the request and determine if it's a good or bad request
      std::cout << "Printing out the mfing request: " << std::endl;
      // add the offset of the carriage_index 
      request_index += carriage_index;
      for(int i = 0; i < request_index; i++){
        std::cout << carriage_buffer[i];
      }
      std::cout << std::endl;
      
      // find the GET in the request header
      int get_index = getDelimeterIndex(carriage_buffer, 0);

      std::cout << "get_index: " << get_index << std::endl;

      // **************************************
      // TODO: check that GET keyword appears
      // **************************************

      // find file name, index is + 1 what the get_index was
      int file_index = getDelimeterIndex(carriage_buffer, get_index + 1);
      std::cout << "file_index: " << file_index<< "get_index: " << get_index << std::endl;

      char fileMsg[file_index - get_index + 1];
      std::cout << "strlen(fileMsg) before population: " << strlen(fileMsg) << std::endl;
      int count = 0;
      for(int i = get_index + 1; i < file_index; i++){
        fileMsg[count] = carriage_buffer[i];
        count++;
        // std::cout << carriage_buffer[i];
      }
      std::cout << "strlen(fileMsg) before printing: " << strlen(fileMsg) << std::endl;
      for(int i = 0; i < strlen(fileMsg); i++) {
        std::cout << fileMsg[i];
      }
      std::cout << std::endl;
      fileMsg[file_index - get_index - 1] = '\0';
      std::cout << "strlen(fileMsg): " << strlen(fileMsg) << std::endl;
      
      // check to see if fileMsg is a valid file name
      // check for html file
      if(strcmp(fileMsg, "/file1.html") == 0) {
        std::cout << "This is a valid html file name" << std::endl;

        // parse correct file name
      } else if(strcmp(fileMsg, "image1.jpg") == 0) { // check for image file
        std::cout << "This is a valid jpg file name" << std::endl;

      }
      else {
        std::cout << "was not a valid file name" << std::endl;
        // Print out 404 error
      }
      exit(0);
    } else {
      std::cout << "We didn't find a carriage, carriage_index: " << carriage_index << std::endl;
      // build the carriage buffer since we still need to keep reading
      for(size_t i = 0; i < buffer_size; i++){
        carriage_buffer[carriage_index + i] = buffer[i];
      }
      carriage_index += buffer_size;
    }
    //
    // Check for one of the commands
    //
    // if(strstr(buffer, "GET")) {
    //     DEBUG << "Recieved a \"GET\" request" << ENDL;

    //     // Parse through the buffer checking for correct format of GET request.
    //     // Need to check that the file is a valid file, if not return a 404 response.
    //     // Need to check that the request is a valid GET request, if not return a 404.
    //     int startingIndex = 0; 
    //     std::cout << "about to print out buffer string" << buffer << std::endl; 
    //     int get_msg_index = getDelimeterIndex(buffer, startingIndex);
    //     std::cout << "get_msg_index: " << get_msg_index << std::endl; 
        
    //     // char *getMsg = getStr(buffer,0,get_msg_index);
    //     char getMsg[get_msg_index];
    //     for(size_t i = 0; i < get_msg_index; i++) {
    //         getMsg[i] = buffer[i];
    //     }
    //     getMsg[get_msg_index] = '\0';
    //     std::cout << "getMsg: " << getMsg << std::endl;
    //     if(strcmp(getMsg, "GET") == 0) {
    //         std::cout << "This is a GET request" << std::endl;
    //     } else {
    //         std::cout << "was not a GET request" << std::endl;
    //         // Print out bad request message
    //     }
    //     int file_request_index = getDelimeterIndex(buffer, get_msg_index + 1);
    //     std::cout << "file_request_index: " << file_request_index << std::endl;
    //     // slice buffer to get index;
    //     // for(size_t i = 4; buffer[i] != ' ' || 0; i++) {
    //        // std::cout << buffer[i] << std::endl;
    //     //}
    //     // Parse through request word for word
    // }
   //  DEBUG << "Writing data to client" << ENDL;
    //
    // Call write() to send line back to the client.
    //
    // write(sockFd, buffer, 1024);
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
  }

  close(listenFd);

}
