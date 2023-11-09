#ifndef USEHANDLER_H
#define USEHANDLER_H

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/Exception.h"
#include "Poco/ThreadPool.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include <iostream>
#include <iostream>
#include <fstream>

using Poco::DateTimeFormat;
using Poco::DateTimeFormatter;
using Poco::ThreadPool;
using Poco::Timestamp;
using Poco::Net::HTMLForm;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::NameValueCollection;
using Poco::Net::ServerSocket;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;

#include "../../database/basket.h"

static bool hasSubstr(const std::string &str, const std::string &substr)
{
    if (str.size() < substr.size())
        return false;
    for (size_t i = 0; i <= str.size() - substr.size(); ++i)
    {
        bool ok{true};
        for (size_t j = 0; ok && (j < substr.size()); ++j)
            ok = (str[i + j] == substr[j]);
        if (ok)
            return true;
    }
    return false;
}

class BasketHandler : public HTTPRequestHandler
{
private:
    bool check_name(const std::string &name, std::string &reason)
    {
        if (name.length() < 3)
        {
            reason = "Name must be at least 3 signs";
            return false;
        }

        return true;
    };

    bool check_quantity(const int &quantity_of_product, std::string &reason)
    {
        if (quantity_of_product <= 0)
        {
            reason = "Quantity must be at least 1";
            return false;
        }

        return true;
    };

    bool check_price(const int &price, std::string &reason)
    {
        if (price < 0)
        {
            reason = "Price must be non-negative";
            return false;
        }

        return true;
    }

    void badRequestError(HTTPServerResponse &response, std::string instance)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/bad_request");
        root->set("title", "Internal exception");
        root->set("status", "400");
        root->set("detail", "Недостаточно параметров в теле запроса");
        root->set("instance", instance);
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

    void notFoundError(HTTPServerResponse &response, std::string instance, std::string message)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/not_found");
        root->set("title", "Internal exception");
        root->set("status", "404");
        root->set("detail", message);
        root->set("instance", instance);
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    };

public:
    BasketHandler(const std::string &format) : _format(format)
    {
    }

    void handleRequest(HTTPServerRequest &request,
                       HTTPServerResponse &response)
    {
        HTMLForm form(request, request.stream());
        try
        {
            if (hasSubstr(request.getURI(), "/basket"))
            {
                if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST)
                {
                    if (form.has("user_id") && form.has("product_id") && form.has("quantity_of_products"))
                    {
                        database::Basket Basket;
                        Basket.user_id() = atol(form.get("user_id").c_str());
                        Basket.product_id() = atol(form.get("product_id").c_str());
                        Basket.quantity_of_products() = stoi(form.get("quantity_of_products"));

                        bool check_result = true;
                        std::string message;
                        std::string reason;

                        if (!check_quantity(Basket.get_quantity_of_products(), reason))
                        {
                            check_result = false;
                            message += reason;
                            message += "<br>";
                        }

                        if (check_result)
                        {
                            Basket.save_to_mysql();
                            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                            response.setChunkedTransferEncoding(true);
                            response.setContentType("application/json");
                            std::ostream &ostr = response.send();
                            ostr << Basket.get_id();
                            return;
                        }
                        else
                        {
                            notFoundError(response, request.getURI(), message);
                            return;
                        }
                    }
                    else
                    {
                        badRequestError(response, request.getURI());
                        return;
                    }
                }
                else if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_PUT)
                {
                    if (form.has("id") && form.has("quantity_of_products"))
                    {
                        database::Basket Basket;
                        Basket.id() = atol(form.get("id").c_str());
                        Basket.quantity_of_products() = stoi(form.get("quantity_of_products"));

                        bool check_result = true;
                        std::string message;
                        std::string reason;

                        if (!check_quantity(Basket.get_quantity_of_products(), reason))
                        {
                            check_result = false;
                            message += reason;
                            message += "<br>";
                        }

                        if (check_result)
                        {
                            Basket.update_in_mysql();
                            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                            response.setChunkedTransferEncoding(true);
                            response.setContentType("application/json");
                            return;
                        }
                        else
                        {
                            notFoundError(response, request.getURI(), message);
                            return;
                        }
                    }
                    else
                    {
                        badRequestError(response, request.getURI());
                        return;
                    }
                }
                else if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE)
                {
                    if (form.has("id") && !form.has("user_id"))
                    {
                        long id = atol(form.get("id").c_str());

                        if (database::Basket::delete_in_mysql_product(id))
                        {
                            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                            response.setChunkedTransferEncoding(true);
                            response.setContentType("application/json");
                            return;
                        }
                        else
                        {
                            notFoundError(response, request.getURI(), "Product with this id not found");
                            return;
                        }
                    }
                    else if(!form.has("id") && form.has("user_id"))
                    {
                        long user_id = atol(form.get("product_id").c_str());

                        if (database::Basket::delete_in_mysql_basket(user_id))
                        {
                            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                            response.setChunkedTransferEncoding(true);
                            response.setContentType("application/json");
                            return;
                        }
                        else
                        {
                            notFoundError(response, request.getURI(), "Basket or user not found");
                            return;
                        }
                    }
                    else
                    {
                        badRequestError(response, request.getURI());
                        return;
                    }
                }
            }
            else if (hasSubstr(request.getURI(), "/all_Baskets"))
            {
                auto results = database::Basket::read_all();
                if (!results.empty())
                {
                    Poco::JSON::Array arr;
                    for (auto s : results)
                        arr.add(s.toJSON());

                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(arr, ostr);
                    return;
                }
                else
                {
                    notFoundError(response, request.getURI(), "Baskets not found");
                    return;
                }
            }
            else if (hasSubstr(request.getURI(), "/search_Basket"))
            {
                if (form.has("user_id"))
                {
                    long fn = atol(form.get("user_id").c_str());
                    auto results = database::Basket::search_by_user_id(fn);
                    if (!results.empty())
                    {
                        Poco::JSON::Array arr;
                        for (auto s : results)
                            arr.add(s.toJSON());
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        std::ostream &ostr = response.send();
                        Poco::JSON::Stringifier::stringify(arr, ostr);
                        return;
                    }
                    else
                    {
                        notFoundError(response, request.getURI(), "Basket not found for this user");
                        return;
                    }
                }
                else
                {
                    badRequestError(response, request.getURI());
                    return;
                }
            }
        }
        catch (const Poco::Exception &e)
        {
            std::cout << e.displayText() << std::endl;
        }
        notFoundError(response, request.getURI(), "request not found");
    }

private:
    std::string _format;
};
#endif