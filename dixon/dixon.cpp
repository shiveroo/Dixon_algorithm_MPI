// dixon.cpp: definiuje punkt wejścia dla aplikacji konsolowej.
//

#include "stdafx.h"
#include <iostream>
#include <algorithm>
#include <ctime>
#include <random>
#include <ostream>
#include <string>
#include <conio.h>
#include <time.h>
#include <stdio.h>
#include <mpi.h>

using namespace std;


int is_smooth(long long int n) {

	long long int fbase[] = { 2, 3, 5, 7, 11, 13, 19, 23 };
	long long int length_fbase = (sizeof(fbase) / sizeof(*fbase));


	for (long long int i = 0; i < sizeof(fbase) / sizeof(fbase[0]); i = i + 1) {

		while (n % fbase[i] == 0) {
			n = n / fbase[i];
			//cout << n << ", " << fbase[i] << endl;
		}
	}

	if (n == 1) {
		return true;
	}
	else {
		return false;
	}
}

int generate_prime(int start) {

	int n, i, p, lp;
	bool t;
	lp = 0;
	p = start;
	n = 1;
	while (lp < n)
	{
		t = true;
		for (i = 2; i < p; i++)
			if (p % i == 0)
			{
				t = false;
				break;
			}
		if (t)
		{
			//cout << p << " ";
			return p;
			lp++;
		}
		p++;
	}

	return false;

}


unsigned GCD(unsigned u, unsigned v) {
	while (v != 0) {
		unsigned r = u % v;
		u = v;
		v = r;
	}
	return u;
}

long long int square_mod_n(long long int number, long long int n) {

	long long int square_mod_n = pow(number, 2);
	square_mod_n = square_mod_n % n;

	return square_mod_n;
}


int factorization(long long int smooth1, long long int smooth2, int n, int proc_id) {

	long long int a = smooth1;
	long long int b = smooth2;

	long long int ab = a * b;
	long long int part1 = ab;

	while (part1 >= n) {

		part1 = part1 - n;
	}

	long long int pow1 = pow(part1, 2);
	long long int part1_pow_mod_n = pow1 % n;
	long long int part2 = 0;

	for (int i = 1; i < part1; i++) {

		long long int pow2 = pow(i, 2);

		if ((pow2 % n) == part1_pow_mod_n) {
			part2 = i;
			break;
		}
	}

	if (part2 == 0)
		return false;

	int factor1 = GCD(part1 - part2, n);
	int factor2 = GCD(part1 + part2, n);

	if (factor1 == 1 || factor2 == 2)
		return false;

	//cout << "Smooth 1: " << smooth1 << endl;
	//cout << "smooth 2: " << smooth2 << endl;
	cout << proc_id << ") " << "Proces " << proc_id << endl;
	cout << "Factor 1: " << factor1 << endl;
	cout << "Factor 2: " << factor2 << endl;

	cout << endl << "The number is: " << factor1 << " x " << factor2 << " = " << factor1 * factor2 << endl;


	return true;
}


int dixon_ver2(long long int n, int proc_id, int numproces) {

	//out << "Dixon ver2" << endl;
	long long int square = ceil(sqrt(n));
	long long int number = square;
	long long int smooth_numbers[32722];
	long long int counter = 0;

	srand(time(NULL));
	long long int timer = 0;

	long long int diffrence = n - square;
	long long int range = floor(diffrence / (numproces - 2));

	int counting_proces = numproces - 2;
	int lastproc = numproces - 1;

	while (factorization(smooth_numbers[0], smooth_numbers[1], n, proc_id) == 0) {

		counter = 0;

		while (counter <= 2) {

			int number = 0;

			if (proc_id == lastproc) {
				number = square + (std::rand() % (n - square + 1));
			}
			else if(proc_id == 1) {
				number = square + (std::rand() % (n - square - (range * (counting_proces - 1)) + 1));
			}
			else {
				
				for (int i = 2; i < numproces; i++) {
					if (i == counting_proces) {
						number = (square + ((counting_proces - 1)) * diffrence) + (std::rand() % (n - square + 1));
					}
					else {
						number = (square + ((proc_id - 1)*diffrence)) % (n - square - ((numproces - proc_id - 2)*diffrence) + 1);
					}
				}	
			};
			
			if (is_smooth(square_mod_n(number, n)) == 1) {
				smooth_numbers[counter] = number;
				//cout << number << ", " << counter << endl;
				counter++;
			}

		}
	}
	return true;
}

void getNumber(string &result, int argc, char *argv[])
{
	result = 1;

	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-n") == 0 && i + 1 != argc) {
			result = argv[i + 1];
		}
	}
}


int main(int argc, char *argv[])
{
	int ierr = MPI_Init(&argc, &argv);

	int procid;
	ierr = MPI_Comm_rank(MPI_COMM_WORLD, &procid);

	int numprocs;
	ierr = MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

	if (numprocs < 2)
	{
		cout << "Error: Number of processes is less than 2!" << endl;
		return MPI_Abort(MPI_COMM_WORLD, 1);
	}

	string number = " ";
	getNumber(number, argc, argv);

	long long int n = atoi(number.c_str());

	srand(NULL);
	int to_factor = floor(n/3) + (std::rand() % (n*2 + 1));

	long long int a = generate_prime(n) * generate_prime(to_factor);

	a = n;

	if (procid == 0)
	{
		cout << "NUMBER: " << a << endl;

		for (int i = 1; i < numprocs; i++) {
			int receiverId = i;
			int sendValue = i;
			MPI_Send(&sendValue, 1, MPI_INT, receiverId, 0, MPI_COMM_WORLD);
		}
		
	}
	else {

		MPI_Status status;
		int receivedValue;

		MPI_Recv(&receivedValue, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
	
		int result = dixon_ver2(a, receivedValue, numprocs);

		if (result == 1) {
			return MPI_Abort(MPI_COMM_WORLD, 1);
		}
		
	}

	ierr = MPI_Finalize();

	return 0;
}

