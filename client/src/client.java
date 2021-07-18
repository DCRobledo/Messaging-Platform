import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.URL;

import gnu.getopt.Getopt;

import client.TextConversorServiceService;
import client.TextConversorService;

class client {
	/********************* TYPES **********************/
	
	
	/******************* ATTRIBUTES *******************/
	private static String _server   = null;
	private static int _port = -1;

	private static String user_name;
	private static clientThread client_thread;
	private static ServerSocket client_socket;
		
	
	/********************* METHODS ********************/
	static ServerSocket get_client_socket() {
		ServerSocket socket = null;

		try {
			socket = new ServerSocket(0);
		} catch (IOException e) {
			System.out.println("Exception " + e);
			e.printStackTrace();
		}

		if (socket == null || socket.getLocalPort() < 0) {
			System.out.println("Error while getting the client's port");
			return null;
		}

		return socket;
	}


	static byte process_request(String ...args) {
		try {
			Socket sc = new Socket(_server, _port);

			DataOutputStream out = new DataOutputStream(sc.getOutputStream());
			DataInputStream in = new DataInputStream(sc.getInputStream());

			out.writeBytes(String.join("\0", args) + '\0');
			
			byte[] ch = new byte[1];
			ch[0] = in.readByte();

			sc.close();

			return ch[0];

		} catch (Exception e) {
			System.err.println("Exception " + e.toString());
			e.printStackTrace();
			return 2;
		}
	}

	static byte[] process_send_message(String ...args) {
		byte[] ch = new byte[255];
		try {
			Socket sc = new Socket(_server, _port);
			DataOutputStream out = new DataOutputStream(sc.getOutputStream());
			DataInputStream in = new DataInputStream(sc.getInputStream());
			out.writeBytes(String.join("\0", args) + '\0');
			ch[0] = in.readByte();
			ch[1] = 0;
			String messageId = in.readLine();
			int cont = 0;
			for (int i = 1; i< messageId.length(); i++) ch[i] = (byte)messageId.charAt(i);
			sc.close();
			return ch;
		} catch (Exception e) {
			e.printStackTrace();
			System.err.println("Error reading the response");
			ch[0] = 2;
			return ch;
		}
	}



	/**
	 * @param user - User name to register in the system
	 * 
	 * @return OK if successful
	 * @return USER_ERROR if the user is already registered
	 * @return ERROR if another error occurred
	 */
	static RC register(String user) 
	{
		byte result = process_request("REGISTER", user);

		RC rc = result == 0 ? RC.OK : result == 1 ? RC.USER_ERROR : RC.ERROR;

		switch (rc) {
			case OK: System.out.println("c> REGISTER OK"); break;

			case USER_ERROR: System.out.println("c> USERNAME IN USE"); break;

			default: System.out.println("c> REGISTER FAIL");
		}
			
		return rc;
	}
	
	/**
	 * @param user - User name to unregister from the system
	 * 
	 * @return OK if successful
	 * @return USER_ERROR if the user does not exist
	 * @return ERROR if another error occurred
	 */
	static RC unregister(String user) 
	{
		byte result = process_request("UNREGISTER", user);

		RC rc = result == 0 ? RC.OK : result == 1 ? RC.USER_ERROR : RC.ERROR;

		switch (rc) {
			case OK: System.out.println("c> UNREGISTER OK"); break;

			case USER_ERROR: System.out.println("c> USER DOES NOT EXIST"); break;

			default: System.out.println("c> UNREGISTER FAIL");
		}
			
		return rc;
	}
	
    /**
	 * @param user - User name to connect to the system
	 * 
	 * @return OK if successful
	 * @return USER_ERROR if the user does not exist or if it is already connected
	 * @return ERROR if another error occurred
	 */
	static RC connect(String user) 
	{
		user_name = user;

		client_socket = get_client_socket();

		byte result = process_request("CONNECT", user, String.valueOf(client_socket.getLocalPort()));
		RC rc = result == 0 ? RC.OK : result == 1 ? RC.USER_ERROR : result == 2 ? RC.CONNECT_ERROR : RC.DEFAULT;

		switch (rc) {
			case OK:
				System.out.println("c> CONNECT OK");

				client_thread = new clientThread(client_socket);
		
				client_thread.start();

				break;

			case USER_ERROR: System.out.println("c> CONNECT FAIL, USER DOES NOT EXIST"); break;

			case CONNECT_ERROR: System.out.println("c> USER ALREADY CONNECTED"); break;

			default: System.out.println("c> CONNECT FAIL");
		}
			
		return rc;
	}
	
	 /**
	 * @param user - User name to disconnect from the system
	 * 
	 * @return OK if successful
	 * @return USER_ERROR if the user does not exist
	 * @return ERROR if another error occurred
	 */
	static RC disconnect(String user) 
	{
		byte result = process_request("DISCONNECT", user);

		RC rc = result == 0 ? RC.OK : result == 1 ? RC.USER_ERROR : result == 2 ? RC.CONNECT_ERROR : RC.DEFAULT;

		switch (rc) {
			case OK: System.out.println("c> DISCONNECT OK"); break;

			case USER_ERROR: System.out.println("c> DISCONNECT FAIL / USER DOES NOT EXIST"); break;

			case CONNECT_ERROR: System.out.println("c> DISCONNECT FAIL / USER NOT CONNECTED"); break;

			default: System.out.println("c> DISCONNECT FAIL");
		}

		if (rc == RC.OK) {
			try {

				client_thread.must_run = false;
				client_socket.close();
				client_thread.join();
				user_name = "";

			} catch (IOException e) {

				System.err.println("Error at disconnect");
				e.printStackTrace();
				return RC.ERROR;

			} catch (Exception e) {
				e.printStackTrace();
			} 
		}
	
		return rc;
	}

	 /**
	 * @param user    - Receiver user name
	 * @param message - Message to be sent
	 * 
	 * @return OK if the server had successfully delivered the message
	 * @return USER_ERROR if the user is not connected (the message is queued for delivery)
	 * @return ERROR the user does not exist or another error occurred
	 */
	static RC send(String user, String message) { // SEND reciever message

		try {

			URL  url = new URL("http://" +_server + ":8888/rs?wsdl");
			TextConversorServiceService service = new TextConversorServiceService(url);
			TextConversorService port = service.getTextConversorServicePort();
			message = port.capitalize(message);

		} catch (Exception e) {
			System.err.println("Exception " + e.toString());
		}

		byte[] result = new byte[255];
		result = process_send_message("SEND", user_name, user, message);

		RC rc = result[0] == 0 ? RC.OK : result[0] == 1 ? RC.USER_ERROR : result[0] == 2 ? RC.SEND_ERROR : RC.DEFAULT;
		String id = "";
		byte cont = 0;
		while (result[++cont] != 0) id += (char)result[cont];

		switch (rc) {
			case OK: System.out.println("c> SEND OK - MESSAGE " + id); break;

			case USER_ERROR: System.out.println("c> SEND FAIL / USER DOES NOT EXIST"); break;

			case SEND_ERROR: System.out.println("c> SEND FAIL"); break;

			default: System.out.println("c> SEND FAIL");
		}
			
		return rc;
	}


	 /**
	 * @param user    - Receiver user name
	 * @param file    - file  to be sent
	 * @param message - Message to be sent
	 * 
	 * @return OK if the server had successfully delivered the message
	 * @return USER_ERROR if the user is not connected (the message is queued for delivery)
	 * @return ERROR the user does not exist or another error occurred
	 */
	static RC sendAttach(String user, String file, String message) 
	{
		// Write your code here
		return RC.ERROR;
	}
	
	
	/**
	 * @brief Command interpreter for the client. It calls the protocol functions.
	 */
	static void shell() 
	{
		boolean exit = false;
		String input;
		String [] line;
		BufferedReader in = new BufferedReader(new InputStreamReader(System.in));

		while (!exit) {
			try {
				System.out.print("c> ");
				input = in.readLine(); 
				line = input.split("\\s");

				if (line.length > 0) {
					/*********** REGISTER *************/
					if (line[0].equals("REGISTER")) {
						if  (line.length == 2) {
							register(line[1]); // userName = line[1]
						} else {
							System.out.println("Syntax error. Usage: REGISTER <userName>");
						}
					} 
					
					/********** UNREGISTER ************/
					else if (line[0].equals("UNREGISTER")) {
						if  (line.length == 2) {
							unregister(line[1]); // userName = line[1]
						} else {
							System.out.println("Syntax error. Usage: UNREGISTER <userName>");
						}
                    } 
                    
                    /************ CONNECT *************/
                    else if (line[0].equals("CONNECT")) {
						if  (line.length == 2) {
							connect(line[1]); // userName = line[1]
						} else {
							System.out.println("Syntax error. Usage: CONNECT <userName>");
                    	}
                    } 
                    
                    /********** DISCONNECT ************/
                    else if (line[0].equals("DISCONNECT")) {
						if  (line.length == 2) {
							disconnect(line[1]); // userName = line[1]
						} else {
							System.out.println("Syntax error. Usage: DISCONNECT <userName>");
                    	}
                    } 
                    
                    /************** SEND **************/
					else if (line[0].equals("SEND")) {
						if  (line.length >= 3) {
							// Remove first two words
							String aux = input.substring(input.indexOf(' ')+1);
							String message = aux.substring(aux.indexOf(' ')+1);
							send(line[1], message); // userName = line[1]
						} else {
							System.out.println("Syntax error. Usage: SEND <userName> <message>");
                    	}
                    }   
                    
                    /************** QUIT **************/
                    else if (line[0].equals("QUIT")){
						if (line.length == 1) {
							//try to disconnect in case of the user is alrredy connected
							if (!user_name.equals("")) disconnect(user_name);
							exit = true;
						} else {
							System.out.println("Syntax error. Use: QUIT");
						}
					} 
					
					/************* UNKNOWN ************/
					else {						
						System.out.println("Error: command '" + line[0] + "' not valid.");
					}
				}				
			} catch (java.io.IOException e) {
				System.out.println("Exception: " + e);
				e.printStackTrace();
			}
		}
	}
	
	/**
	 * @brief Prints program usage
	 */
	static void usage() 
	{
		System.out.println("Usage: java -cp . client -s <server> -p <port>");
	}
	
	/**
	 * @brief Parses program execution arguments 
	 */ 
	static boolean parseArguments(String [] argv) 
	{
		Getopt g = new Getopt("client", argv, "ds:p:");

		int c;
		String arg;

		while ((c = g.getopt()) != -1) {
			switch(c) {
				case 's':
					_server = g.getOptarg();
					break;
				case 'p':
					arg = g.getOptarg();
					_port = Integer.parseInt(arg);
					break;
				case '?':
					System.out.print("getopt() returned " + c + "\n");
					break; // getopt() already printed an error
				default:
					System.out.print("getopt() returned " + c + "\n");
			}
		}
		
		if (_server == null)
			return false;
		
		if ((_port < 1024) || (_port > 65535)) {
			System.out.println("Error: Port must be in the range 1024 <= port <= 65535");
			return false;
		}

		return true;
	}
	
	
	
	/********************* MAIN **********************/
	
	public static void main(String[] argv) 
	{
		if(!parseArguments(argv)) {
			usage();
			return;
		}
		
		shell();

		System.out.println("+++ FINISHED +++");
	}
}
