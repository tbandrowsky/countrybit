

#include "table.h"
#include "string_box.h"
#include "assert_if.h"

namespace countrybit
{
    namespace database
    {
        bool table_tests()
        {
            int r = true;
            using countrybit::database::istring;
            using countrybit::database::table;
            using countrybit::database::row_range;
            using countrybit::database::static_box;

            struct test_item {
                int id;
                istring<10> name;
                istring<50> description;
            };

            test_item objects[5] = {
                { 0, "hello", "long hello" },
                { 1, "goodbye", "long goodbye"},
                { 2, "yes", "you say yes"},
                { 3, "no", "i say no"},
                { 4, "don't", "i don't why you say goodbye"}
            };

            int s = sizeof(objects) / sizeof(test_item);
            r = r && assert_if([s]() { return s == 5; }, "size isn't 5");

            using box_type = static_box<10000>;

            box_type box;

            table<test_item> basic;

            auto location = table<test_item>::reserve_table(&box, 20);
            basic = table<test_item>::get_table(&box, location);

            test_item* ti = &objects[0];

            for (int i = 0; i < s; i++) {
                row_range rr;
                auto nr = basic.append(objects[i], rr);
                r = r && assert_if([nr, i, rr]() { return rr.stop - rr.start == 1; }, "size isn't 1");
                r = r && assert_if([nr, i, ti]() { return ti[i].id == nr.id && ti[i].description == nr.description && ti[i].name == nr.name; }, "item not stored correctly");
            }

            for (countrybit::database::row_id_type i = 0; i < basic.size(); i++) {
                auto nr = basic[i];
                r = r && assert_if([nr, i, ti]() { return ti[i].id == nr.id && ti[i].description == nr.description && ti[i].name == nr.name; }, "item not stored correctly");
            }

            return r;
        }

    }
}


