#include "HttpProtocol.h"

#include "Util.h"

static const std::string cSupportedMethods[] = { "GET", "POST" };


HttpProtocol::HttpProtocol()
{

}


void HttpProtocol::ParseUrlParameters(HttpRequest &request)
{
    std::istringstream iss(request.query);

    std::string url;

    if(!std::getline(iss, url, '?')) // remove the URL part
    {
        return;
    }

    // store query key/value pairs in a map
    std::string keyval, key, val;

    while(std::getline(iss, keyval, '&')) // split each term
    {
        std::istringstream iss(keyval);

        // split key/value pairs
        if(std::getline(std::getline(iss, key, '='), val))
        {
            request.params[key] = val;
        }
    }
}

bool HttpProtocol::ParseHeader(const std::string &payload, HttpRequest &request)
{
 //   std::string request = "GET /index.asp?param1=hello&param2=128 HTTP/1.1";
    // Request-Line   = Method SP Request-URI SP HTTP-Version CRLF

    std::string line;
    std::istringstream iss(payload);

    bool valid = false;

    // separate the first 3 main parts
    if (std::getline(iss, line))
    {
        line.pop_back(); // remove \r
        std::vector<std::string> parts = Util::Split(line, " ");

        if (parts.size() == 3)
        {
            request.method = parts[0];
            request.query = parts[1];
            request.protocol = parts[2];


            for (const auto & s : cSupportedMethods)
            {
                if (request.method == s)
                {
                    valid = true;
                }
            }
        }
    }

    if (valid)
    {
        // Parse optional URI parameters
        ParseUrlParameters(request);

        // Continue parsing the header
        std::string::size_type index;

        while (std::getline(iss, line))
        {
            if (line != "\r")
            {
                line.pop_back();
                index = line.find(':', 0);
                if(index != std::string::npos)
                {
                    // Convert all header options to lower case (header params are case insensitive in the HTTP spec
                    std::string option = line.substr(0, index);
                    std::transform(option.begin(), option.end(), option.begin(), ::tolower);
                    request.headers.insert(std::make_pair(option, line.substr(index + 1)));
                }
            }
            else
            {
                break; // detected HTTP separator \r\n between header and body
            }
        }

        uint32_t body_start = static_cast<uint32_t>(iss.tellg());
        if (body_start < payload.length())
        {
            request.body = payload.substr(body_start);
        }
       // std::cout << request.body << std::endl;
    /*
        for(auto& kv: m) {
            std::cout << "KEY: `" << kv.first << "`, VALUE: `" << kv.second << '`' << std::endl;
        }

        std::cout << "protocol: " << header.protocol << '\n';
        std::cout << "method  : " << header.method << '\n';
        std::cout << "query   : " << header.query << '\n';
    */
    }

    return valid;
}


std::string HttpProtocol::GenerateRequest(const HttpRequest &request)
{
    std::stringstream ss;

    ss << request.method << " " << request.query << " " << request.protocol << "\r\n";

    for(auto const& x : request.headers )
    {
        ss << x.first << ": " << x.second << "\r\n";
    }

    ss << "\r\n";
    ss << request.body;

    return ss.str();
}

std::string HttpProtocol::GenerateHttpJsonResponse(const std::string &data)
{
    std::stringstream ss;

    ss << "HTTP/1.1 200 OK\r\n";
    ss << "Content-type: application/json\r\n";
    ss << "Content-length: " << data.size() << "\r\n\r\n";
    ss << data;

    return ss.str();
}