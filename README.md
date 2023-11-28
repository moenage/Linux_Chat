# Linux Chatbot

Created a simple "chat"-like facility that enables a user at one terminal to communicate with a user at another terminal.

Two LINUX processes (might not be on the same machine). Each one is started by one of the people who want to talk.

In each process there are 4 threads that each accomplish a different task, all four threads will share access to a list ADT.
Here are there individual tasks:
1) Awaits input from the keyboard
2) Awaits a UDP datagram from another process
3) Prints messages (sent and received)
4) Sends data to the remote UNIX process over the network using UDP

Moreover the messages are encrypted using cipher encryption. This programm can be used locally on the same machine by localhost. Also supports copy/paste and up to 4k characters.
