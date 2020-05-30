#include <iostream>
#include <fstream>

using namespace std;

class ServoBlaster
{
private:
    ofstream servoblaster;
    const string kFilename = "/dev/servoblaster";

public:
    ServoBlaster() {}

    void set_angle(int servo_id, int val)
    {
        servoblaster.open(kFilename, ios::out);

        if (val > 0)
        {
            // Signal to the servoblaster that val is positive.
            servoblaster << "P1-" << servo_id << "=+" << val << endl;
        }
        else
        {
            // Signal to the servoblaster that val is negative.
            servoblaster << "P1-" << servo_id << "=" << val << endl;
        }

        servoblaster.close();
    }
};
