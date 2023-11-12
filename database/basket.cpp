#include "basket.h"
#include "database.h"
#include "../config/config.h"

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

#include <sstream>
#include <exception>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database
{
    void Basket::init()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement create_stmt(session);
            create_stmt << "CREATE TABLE IF NOT EXISTS `Basket` ("
                        << "`id` INT NOT NULL AUTO_INCREMENT,"
                        << "`user_id` INT NOT NULL,"
                        << "`product_id` INT NOT NULL,"
                        << "`quantity_of_products` INT NOT NULL,"
                        << "PRIMARY KEY (`id`),"
                        << "FOREIGN KEY (`user_id`) REFERENCES `user` (`id`),"
                        << "FOREIGN KEY (`product_id`) REFERENCES `Product` (`id`)"
                        << ");",
                now;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    void Basket::update_in_mysql()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement update(session);

            update << "UPDATE `Basket`"
                   << "SET `quantity_of_products` = ? "
                   << "WHERE `id` = ?",
                use(_quantity_of_products),
                use(_id);

            update.execute();

            std::cout << "updated: " << _id << std::endl;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    bool Basket::delete_in_mysql_product(long id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement del(session);
            
            del << "DELETE FROM `Basket` WHERE `id` = ?;",
                use(id),
                range(0, 1);
            del.execute();
            std::cout << "deleted: " << id << std::endl;
            return true;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
        return false;
    }

    bool Basket::delete_in_mysql_basket(long user_id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement del(session);

            del << "DELETE FROM `Basket` WHERE `user_id` = ?;",
                use(user_id),
                range(0, 1);
            del.execute();
            std::cout << "deleted: " << user_id << std::endl;
            return true;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
        return false;
    }

    std::optional<Basket> Basket::check_presence_of_product_in_basket(long user_id, long product_id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            Basket a;
            select << "SELECT `id`, 'quantity_of_products' FROM `Basket` WHERE `user_id` = ? and `product_id` = ?;",
                into(a._id),
                into(a._quantity_of_products),
                use(user_id),
                use(product_id),
                range(0, 1);

            select.execute();
            Poco::Data::RecordSet rs(select);
            if (rs.moveFirst())
                return a;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {
            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }

        return {};
    }

    Poco::JSON::Object::Ptr Basket::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("user_id", _user_id);
        root->set("product_id", _product_id);
        root->set("quantity_of_products", _quantity_of_products);

        return root;
    }

    Basket Basket::fromJSON(const std::string &str)
    {
        Basket Basket;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        Basket.id() = object->getValue<long>("id");
        Basket.user_id() = object->getValue<long>("user_id");
        Basket.product_id() = object->getValue<long>("product_id");
        Basket.quantity_of_products() = object->getValue<int>("quantity_of_products");

        return Basket;
    }

    std::vector<Basket> Basket::read_all()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<Basket> result;
            Basket a;
            select  << "SELECT `id`, `user_id`, `product_id`, `quantity_of_products`" 
                    << "FROM `Basket`",
                into(a._id),
                into(a._user_id),
                into(a._product_id),
                into(a._quantity_of_products),
                range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                if (select.execute())
                    result.push_back(a);
            }
            return result;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::vector<Basket> Basket::search_by_user_id(long user_id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<Basket> result;
            Basket a;
            select << "SELECT id, user_id, product_id, quantity_of_products FROM Basket where user_id = ?",
                into(a._id),
                into(a._user_id),
                into(a._product_id),
                into(a._quantity_of_products),
                use(user_id),
                range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                if (select.execute())
                    result.push_back(a);
            }
            return result;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    void Basket::save_to_mysql()
    {

        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);

            insert << "INSERT INTO Basket (user_id, product_id, quantity_of_products) VALUES(?, ?, ?)",
                use(_user_id),
                use(_product_id),
                use(_quantity_of_products),

            insert.execute();

            Poco::Data::Statement select(session);
            select << "SELECT LAST_INSERT_ID()",
                into(_id),
                range(0, 1); //  iterate over result set one row at a time

            if (!select.done())
            {
                select.execute();
            }
            std::cout << "inserted:" << _id << std::endl;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    long Basket::get_id() const
    {
        return _id;
    }

    const long &Basket::get_user_id() const
    {
        return _user_id;
    }

    const long &Basket::get_product_id() const
    {
        return _product_id;
    }

    const int &Basket::get_quantity_of_products() const
    {
        return _quantity_of_products;
    }

    long &Basket::id()
    {
        return _id;
    }

    long &Basket::user_id()
    {
        return _user_id;
    }

    long &Basket::product_id()
    {
        return _product_id;
    }

    int &Basket::quantity_of_products()
    {
        return _quantity_of_products;
    }
}