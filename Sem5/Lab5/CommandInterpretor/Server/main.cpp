#include <iostream>
#include "Server.h"

int main() {
	setlocale(LC_ALL, "Ru");
	if (start()) {
		std::cout << "Running server ...\n";
	}

	return 0;
}
