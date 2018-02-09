// Copyright (C) by Ashton Mason. See LICENSE.txt for licensing information.

#include <stdio.h>
#include <string>
#include <chrono>

#include <Theron/Theron.h>

#define SLEEP_TIME 2000 // ms


class MyPing : public Theron::Actor
{
public:
	// Constructor, passes the framework to the base class.
	MyPing(Theron::Framework &framework, Theron::Receiver& receiver) : Theron::Actor(framework)
	{
		m_prtReceiver = &receiver;
		// Register the message handler.
		RegisterHandler(this, &MyPing::Print);
	}

private:
	// Handler for messages of type std::string.
	// And then, send the msg to receiver
	void Print(const std::string &message, const Theron::Address from)
	{
		// Print the string.
		printf("\nMyPing: %s\n", message.c_str());

		Theron::Framework &framework = GetFramework();
		if (!framework.Send(
			message,
			this->GetAddress(),
			m_prtReceiver->GetAddress()))
		{
			printf("MyPing: ERROR - Failed to send message\n");
		}
	}

private:
	Theron::Receiver* m_prtReceiver;
};


// Actor type that prints strings.
// Derives from Theron::Actor.
class MyPong : public Theron::Actor
{
public:
	// Constructor, passes the framework to the baseclass.
	MyPong(Theron::Framework &framework) : Theron::Actor(framework)
	{
		// Register the message handler.
		RegisterHandler(this, &MyPong::Print);
	}

private:
	// Handler for messages of type std::string.
	void Print(const std::string &message, const Theron::Address from)
	{
		// Print the string.
		printf("MyPong: %s\n", message.c_str());

		Sleep(SLEEP_TIME);
		time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::string pongMsg = message + " ~ Awesome ~ "  + std::to_string(tt);
		Send(pongMsg, from);
	}
};


struct HelloMessage
{
	char* str;
	int val;
};

// Actor type that prints some values.
// Derives from Theron::Actor.
class MyPrinter: public Theron::Actor
{
public:
	// Constructor, passes the framework to the base class.
	MyPrinter(Theron::Framework &framework) : Theron::Actor(framework)
	{
		// Register the message handler.
		RegisterHandler(this, &MyPrinter::Print);
	}
private:
	// Handler for messages of type HelloMessage.
	void Print(const HelloMessage &message, const Theron::Address from)
	{
		// Print the string.
		printf("\nstr = %s\nval = %d\n", message.str, message.val);
		delete[] message.str;

		// Send a dummy message back for synchronization.
		Send(0, from);
	}
};


int main()
{
	// Construct a framework and instantiate a MyPong within it.
	Theron::Framework framework;
	MyPong myPong(framework);

	Theron::Receiver receiver;

	// Construct a MyPing to receive the reply message.
	MyPing myPing(framework, receiver);

	// Send a string message to the MyPong.
	// We pass the address of the myPing as the 'from' address.
	time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::string strMsg = std::string("Hello World ~ ") + std::to_string(tt);
	if (!framework.Send(
		strMsg,
		myPing.GetAddress(),
		myPong.GetAddress()))
	{
		printf("ERROR: Failed to send message\n");
	}

	receiver.Wait();

	////////////////////////////////////

	MyPrinter myPrinter(framework);
	HelloMessage msg;
	int len = 10;
	msg.str = new char[len]();
	char* tmp = "Hi baby~";
	memcpy(msg.str, tmp, strlen(tmp));
	msg.val = 798;
	// Send a string message to the MyPrinter.
	// We pass the address of the receiver as the 'from' address.
	if (!framework.Send(
		msg,
		receiver.GetAddress(),
		myPrinter.GetAddress()))
	{
		printf("ERROR: Failed to send message\n");
	}

	// Synchronize with the dummy message sent in reply to make sure we're done.
	receiver.Wait();

	getchar();
	Sleep(SLEEP_TIME/2);
}

