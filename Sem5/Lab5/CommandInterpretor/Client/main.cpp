#include <iostream>
#include "Client.h"

int main() {
	setlocale(LC_ALL, "Ru");
	Client cli = Client();
	cli.connect_to_server(10000);
}