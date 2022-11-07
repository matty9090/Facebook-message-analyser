#include <fstream>
#include <string>
#include <iostream>
#include <map>

#include "picojson.h"

struct Message
{
    std::string Person;
    std::string Msg;
    double Reactions;
    double NumLaughing;
    int Messages;
};

int main(int argc, char* argv[])
{
    std::vector<std::string> rawfiles;

    for (int i = 1; i < argc; ++i)
    {
        std::ifstream file(argv[i]);

        if (file.is_open())
        {
            int numlines = 0;
            std::string raw, line;

            while (std::getline(file, line))
            {
                raw += line;
                ++numlines;
            };

            rawfiles.push_back(raw);
            file.close();

            std::cout << "Read " << argv[i] << " (" << numlines << " lines)" << std::endl;
        }
        else
        {
            std::cout << "Could not open " << argv[i] << std::endl;
        }
    }

    std::cout << std::endl;

    std::vector<Message> Results;
    std::map<std::string, int> Funniest;
    std::map<std::string, int> MostMessages;

    for (const std::string& raw : rawfiles)
    {
        picojson::value v;
        std::string err = picojson::parse(v, raw);

        if (err == "")
        {
            picojson::array messages = v.get<picojson::object>()["messages"].get<picojson::array>();

            for (picojson::value msg : messages)
            {
                picojson::object msgobj = msg.get<picojson::object>();

                if (msgobj.find("reactions") != msgobj.end())
                {
                    picojson::array reactions = msgobj["reactions"].get<picojson::array>();

                    double num_laughing = 0.0;

                    for (picojson::value react : reactions)
                    {
                        if (react.get<picojson::object>()["reaction"].get<std::string>() == "\\u00f0\\u009f\\u0098\\u0086")
                        {
                            num_laughing++;
                        }
                    }

                    Results.push_back({
                        msgobj["sender_name"].get<std::string>(),
                        msgobj["content"].is<std::string>() ? msgobj["content"].get<std::string>() : "N/A",
                        static_cast<double>(reactions.size()), num_laughing });

                    Funniest[msgobj["sender_name"].get<std::string>()] += reactions.size();
                    
                }

                MostMessages[msgobj["sender_name"].get<std::string>()]++;
            }
        }
        else
        {
            std::cout << "Error parsing json: " << err << std::endl;
        }
    }

    std::sort(Results.begin(), Results.end(), [](const Message& x, const Message& y) {
        return x.Reactions > y.Reactions;
    });

    const int max = 10;

    for (int i = 0; i < Results.size() && i < max; ++i)
    {
        std::cout << Results[i].Reactions << " - " << Results[i].Msg << " (" << Results[i].Person << ")\n";
    }

    std::cout << std::endl;

    std::vector<Message> FunniestSorted;

    for (const auto& R : Funniest)
    {
        FunniestSorted.push_back({ R.first, "", static_cast<double>(R.second) / MostMessages[R.first] });
    }

    std::sort(FunniestSorted.begin(), FunniestSorted.end(), [](const Message& x, const Message& y) {
        return y.Reactions < x.Reactions;
    });

    for (const auto& R : FunniestSorted)
    {
        std::cout << R.Person << " - " << R.Reactions << std::endl;
    }
}
