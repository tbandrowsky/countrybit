

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

            using box_type = dynamic_box;

            box_type box;

            box.init(50000);

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

            auto count = basic.count_if([](auto& t) { return t.name == "yes" || t.name == "no";  });
            assert_if([count]() { return count == 2; }, "Wrong count");

            auto items = basic.where([](auto& t) { return t.name == "yes" || t.name == "no";  });
            count = 0;
            for (auto r : items) 
            {
                auto* rti = &r.item;
                count++;
            }
            assert_if([count]() { return count == 2; }, "Wrong count");

            for (countrybit::database::row_id_type i = 0; i < basic.size(); i++) {
                auto nr = basic[i];
                r = r && assert_if([nr, i, ti]() { return ti[i].id == nr.id && ti[i].description == nr.description && ti[i].name == nr.name; }, "item not stored correctly");
            }

            row_id_type table_location;
            row_id_type simple_test_location;
            auto simple_test = item_details_table<test_item, object_name>::create_table(&box, 10, 100, simple_test_location);

            auto new_item = simple_test.create_item(2, nullptr);
            new_item.item().name = "test1";
            new_item.item().description = "test1 description";
            new_item.item().id = 1;
            auto idx = new_item.row_id();

            auto pi = simple_test[idx].item();
            bool all_good;
            all_good = pi.name == "test1" && pi.description == "test1 description" && pi.id == 1;
            assert_if([all_good]() { return all_good;  }, "table didn't store round trip");

            auto ppi = simple_test[idx].pitem();
            all_good = ppi->name == "test1" && ppi->description == "test1 description" && ppi->id == 1;
            assert_if([all_good]() { return all_good;  }, "table didn't store round trip ptr");

            auto py = simple_test[idx];
            auto px = &py;
            std::cout << " child count: " << px->size() << std::endl;
            assert_if([px]() { return px->size() == 2; }, "detail count incorrect");

            object_name detail = "child 1";
            simple_test.append_detail(idx, 1, &detail);
            detail = "child 2";
            simple_test.append_detail(idx, 1, &detail);

            py = simple_test[idx];
            px = &py;
            std::cout << " child count: " << px->size() << std::endl;
            assert_if([px]() { return px->size() == 2; }, "detail count incorrect");

            for (int i = 0; i < px->size(); i++) {
                std::cout << px->detail(i) << std::endl;
            }

            item_details_table<test_item, object_name> item_stuff;
            item_stuff = item_details_table<test_item, object_name>::create_table(&box, 10, 100, table_location);

            row_id_type ids[4];

            new_item = item_stuff.create_item(1, nullptr);
            new_item.item().name = "first";
            new_item.item().description = "first description";
            new_item.item().id = 1;
            ids[0] = new_item.row_id();

            new_item = item_stuff.create_item(1, nullptr);
            new_item.item().name = "second";
            new_item.item().description = "second description";
            new_item.item().id = 2;
            ids[1] = new_item.row_id();

            new_item = item_stuff.create_item(1, nullptr);
            new_item.item().name = "third";
            new_item.item().description = "third description";
            new_item.item().id = 3;
            ids[2] = new_item.row_id();

            new_item = item_stuff.create_item(1, nullptr);
            new_item.item().name = "fourth";
            new_item.item().description = "fourth description";
            new_item.item().id = 4;
            ids[3] = new_item.row_id();

            for (int xid = 0; xid < 4; xid++)
            {
                auto testx = ids[xid];
                for (int jid = 0; jid < 8; jid++)
                {
                    object_name test = "test child " + std::to_string(xid) + "," + std::to_string(jid);
                    item_stuff.append_detail(testx, 1, &test);
                }
            }

            for (int xid = 0; xid < 4; xid++)
            {
                auto testx = ids[xid];
                auto tc = item_stuff[testx];
                for (int jid = 0; jid < tc.size(); jid++)
                {
                    auto& child = tc.detail(jid);
                    object_name test = "test child " + std::to_string(xid) + "," + std::to_string(jid);
                    
                    if (child != test) {
                        std::cout << " test didn't match " << xid << ", " << jid << std::endl;
                        r = false;
                    }
                }
            }

            return r;
        }

    }
}


