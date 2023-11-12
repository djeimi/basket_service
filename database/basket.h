#ifndef AUTHOR_H
#define AUTHOR_H

#include <string>
#include <vector>
#include "Poco/JSON/Object.h"
#include <optional>

namespace database
{
    class Basket
    {
    private:
        long _id;
        long _user_id;
        long _product_id;
        int _quantity_of_products;

    public:
        static Basket fromJSON(const std::string &str);

        long get_id() const;
        const long &get_user_id() const;
        const long &get_product_id() const;
        const int &get_quantity_of_products() const;

        long &id();
        long &user_id();
        long &product_id();
        int &quantity_of_products();

        static void init();
        void update_in_mysql();
        static std::optional<Basket> check_presence_of_product_in_basket(long user_id, long product_id);
        static bool delete_in_mysql_product(long id);
        static bool delete_in_mysql_basket(long user_id);
        static std::vector<Basket> read_all();
        static std::vector<Basket> search_by_user_id(long user_id);
        void save_to_mysql();

        Poco::JSON::Object::Ptr toJSON() const;
    };
}

#endif