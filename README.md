# Web Client. Communication with REST API

## The description
This project focuses on developing a C/C++ client application that interacts with a REST API. The client provides a command line interface (CLI) for users to input commands, which are then translated into requests sent to the server. The server, in this case, simulates a virtual library and exposes a REST API for various operations.

## The structure
The project consists of the following files:

- **client.c**         : Implementation of the web client
- **request.c**        : Implementation of the request functions
- **request.h**        : Header file for request functions
- **helpers.c**        : Implementation of helper functions for server operations
- **helpers.h**        : Header file for helper functions
- **buffer.c**         : Implementation of buffer-related functions
- **buffer.h**         : Header file for buffer-related functions

## The main commands for User Input
The program reads each command in an infinite loop while opening a connection with the server. The accepted commands from the user include:

1. **exit**: Terminate the program.
2. **register**: Register a new user.
3. **login**: Authenticate a user.
4. **logout**: Disconnect the client.
5. **enter_library**: Gain access to the library.
6. **get_books**: Retrieve all books.
7. **get_book**: Get information about a specific book.
8. **add_book**: Add a book to the library.
9. **delete_book**: Delete a book from the library.

If an unrecognized command is entered, an error message is displayed.

### register
For registration, the program reads the username and password, utilizing the get_user_infos() function, which is also used for login. A POST request message is created and sent to the server, and the following errors are handled:

- The username has already been used.
- The username contains whitespace.

In case of a successful registration, the user is notified.

### login
The program reads the login credentials and parses them using the get_user_infos() function. A POST request message is created, and the following errors are handled based on the server response:

- The credentials do not match.
- The username contains whitespace.

In case of a successful login, the user is notified, and the obtained cookie is stored.

### logout
Since no user input is required for logout, a POST request message is created directly. The following error is handled based on the server response:

- The user was not logged in.

In case of a successful logout, the user is notified.

### enter_library
A GET request message is created to gain access to the library, and the following error is handled based on the server response:

- Unauthorized access.

In case of successful access, the user is notified, and the obtained token is stored.

### get_books
A GET request message is created to retrieve the list of books, and the following error is handled based on the server response:

- Unauthorized access.

In case of a successful request, the JSON list of books is displayed.

### get_book
The program obtains the book ID from the user using the get_id() function and generates a new path. A GET request message is created, and the following errors are handled based on the server response:

- Unauthorized access.
- The ID is in an invalid format or was not entered.
- The book was not found in the library.

In case of a successful request, the details about the book are displayed.

### add_book
The program reads the information about the book and creates a POST request message. The following errors are handled based on the server response:

- Unauthorized access.
- Incomplete or incorrectly formatted data.

In case of a successful request, a success message is displayed.

### delete_book
The program obtains the book ID from the user using the get_id() function and generates a new path. A DELETE request message is created, and the following errors are handled based on the server response:

- Unauthorized access.
- The ID is in an invalid format or was not entered.
- The book was not found in the library.

In case of a successful request, a success message is displayed.
