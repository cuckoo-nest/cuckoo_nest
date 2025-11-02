#include <string>
class Beeper 
{
public:
    Beeper(std::string device_path);
    ~Beeper();
    void play(int duration_ms);

private:
    std::string device_path_;
};