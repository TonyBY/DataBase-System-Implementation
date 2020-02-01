#include <iostream>
#include <list>
 
using namespace std;
const int MAX = 4;
 
int main ()
{
/**
 *  Pointer Study Area
 **/
 const char *names[MAX] = {
                   "Zara Ali",
                   "Hina Ali",
                   "Nuha Ali",
                   "Sara Ali",
   };

int numbers[MAX] = {0, 1, 2, 3,};
int *ptr[MAX];
 
   for (int i = 0; i < MAX; i++)
   {
      cout << "Value of names[" << i << "] = ";
      cout << names + i << endl;
      cout << &names[i] << endl;
      cout << *names[i] << endl;
      cout << names[i] << endl;
   }

    cout << "----------------" << endl;
    cout << "----------------" << endl;
   for (int i = 0; i < MAX; i++)
   {
        ptr[i] = &numbers[i];
        cout << "Value of numbers[" << i << "] = ";
        cout << numbers + i << endl;
        cout << &numbers[i] << endl;
        cout << numbers[i] << endl;
        cout << ptr[i] << endl;
        cout << *ptr[i] << endl;
   }
   return 0;
}