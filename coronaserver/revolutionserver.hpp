
#include "corona.hpp"

#include <windows.h>
#include <iostream>

namespace corona::apps
{   
    class revolution_server
    {
    public:

        bool operator()(comm_bus_service* _service, json& _command)
        {
            std::string class_name = _command["class_name"];

            if (class_name == "")
            {
                ;
            }
            else if (class_name == "")
            {
                ;
            }
            else if (class_name == "")
            {
                ;
            }
            else if (class_name == "")
            {
                ;
            }

            return true;
        }
    };

}
