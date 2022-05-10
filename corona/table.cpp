

#include "table.h"
#include "string_box.h"
#include "assert_if.h"

namespace corona
{
    namespace database
    {
        bool table_tests()
        {
            int r = true;
            using corona::database::istring;
            using corona::database::table;
            using corona::database::row_range;
            using corona::database::static_box;

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

            test_item moved_foward_objects[6] = {
    { 0, "hello", "long hello" },
    { 0, "hello", "long hello" },
    { 1, "goodbye", "long goodbye"},
    { 2, "yes", "you say yes"},
    { 3, "no", "i say no"},
    { 4, "don't", "i don't why you say goodbye"}
            };

            test_item moved_backward_objects[5] = {
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
            test_item* tif = &moved_foward_objects[0];
            test_item* tib = &moved_backward_objects[0];

            for (int i = 0; i < s; i++) {
                row_range rr;
                auto nr = basic.append(objects[i], rr);
                r = r && assert_if([nr, i, rr]() { return rr.stop - rr.start == 1; }, "size isn't 1");
                r = r && assert_if([nr, i, ti]() { return ti[i].id == nr.id && ti[i].description == nr.description && ti[i].name == nr.name; }, "item not stored correctly");
            }

            auto count = basic.count_if([](auto& t) { return t.item.name == "yes" || t.item.name == "no";  });
            assert_if([count]() { return count == 2; }, "Wrong count");

            auto items = basic.where([](auto& t) { return t.item.name == "yes" || t.item.name == "no";  });
            count = 0;
            for (auto r : items) 
            {
                auto* rti = &r.item;
                count++;
            }
            assert_if([count]() { return count == 2; }, "Wrong count");

            for (corona::database::relative_ptr_type i = 0; i < basic.size(); i++) {
                auto nr = basic[i];
                r = r && assert_if([nr, i, ti]() { return ti[i].id == nr.id && ti[i].description == nr.description && ti[i].name == nr.name; }, "item not stored correctly");
            }

#if DETAILS
            std::cout << " insert" << std::endl;
#endif

            basic.insert(0, 1);

#if DETAILS

            for (corona::database::relative_ptr_type i = 0; i < basic.size(); i++) {
                auto b = basic[i];
                std::cout << b.id << " " << b.name << " " << b.description << std::endl;
            }
#endif
            for (corona::database::relative_ptr_type i = 0; i < basic.size(); i++) {
                auto nr = basic[i];
                r = r && assert_if([nr, i, tif]() { return tif[i].id == nr.id && tif[i].description == nr.description && tif[i].name == nr.name; }, "forward not moved correctly");
            }
            
#if DETAILS
            std::cout << " remove" << std::endl;
#endif

            basic.erase(0, 1);

#if DETAILS
            for (corona::database::relative_ptr_type i = 0; i < basic.size(); i++) {
                auto b = basic[i];
                std::cout << b.id << " " << b.name << " " << b.description << std::endl;
            }
#endif

            for (corona::database::relative_ptr_type i = 0; i < basic.size(); i++) {
                auto nr = basic[i];
                r = r && assert_if([nr, i, tib]() { return tib[i].id == nr.id && tib[i].description == nr.description && tib[i].name == nr.name; }, "backward not moved correctly");
            }

            relative_ptr_type table_location;
            relative_ptr_type simple_test_location;
            auto simple_test = item_details_table<test_item, object_name>::create_table(&box, 10, 100, simple_test_location);

            test_item tix;
            tix.name = "test1";
            tix.description = "test1 description";
            tix.id = 1;

            auto new_item = simple_test.create_item(&tix, 2, nullptr);
            auto idx = new_item.row_id();
            auto pi = simple_test[idx].item();
            bool all_good;
            all_good = pi.name == "test1" && pi.description == "test1 description" && pi.id == 1;
            assert_if([all_good]() { return all_good;  }, "table didn't store round trip");

            new_item.item().name = "test1";
            new_item.item().description = "test1 description";
            new_item.item().id = 1;

            pi = simple_test[idx].item();
            all_good;
            all_good = pi.name == "test1" && pi.description == "test1 description" && pi.id == 1;
            assert_if([all_good]() { return all_good;  }, "table didn't store round trip");

            auto ppi = simple_test[idx].pitem();
            all_good = ppi->name == "test1" && ppi->description == "test1 description" && ppi->id == 1;
            assert_if([all_good]() { return all_good;  }, "table didn't store round trip ptr");

            auto py = simple_test[idx];
            auto px = &py;
            assert_if([px]() { return px->size() == 0; }, "detail count incorrect");

            object_name detail = "child 1";
            simple_test.append_detail(idx, 1, &detail);
            detail = "child 2";
            simple_test.append_detail(idx, 1, &detail);

#if DETAILS

            py = simple_test[idx];
            px = &py;

            std::cout << " child count: " << px->size() << std::endl;
            assert_if([px]() { return px->size() == 2; }, "detail count incorrect");

            for (int i = 0; i < px->size(); i++) {
                std::cout << px->detail(i) << std::endl;
            }

#endif

            relative_ptr_type insert_test2_location;
            auto insert_test2 = table<test_item>::create_table(&box, 20, insert_test2_location);

            tix.name = "test 1";
            tix.description = "test1 description";
            tix.id = 1;
            insert_test2.create(1, &tix);
            tix.name = "test 2";
            tix.description = "test2 description";
            tix.id = 1;
            insert_test2.create(1, &tix);

            insert_test2.insert(1, 3);
            for (relative_ptr_type x = 1; x < 4; x++)
            {
                auto* tic = insert_test2.get_ptr(x);
                tic->name = "test x";
                tic->description = "testx description";
                tic->id = 1;
            }

#if DETAILS
            for (int i = 0; i < insert_test2.size(); i++) 
            {
                std::cout << insert_test2[i].name << std::endl;
            }
#endif

            item_details_table<test_item, object_name> item_stuff;
            item_stuff = item_details_table<test_item, object_name>::create_table(&box, 10, 100, table_location);

            relative_ptr_type ids[4];

            new_item = item_stuff.create_item(nullptr, 1, nullptr);
            new_item.item().name = "first";
            new_item.item().description = "first description";
            new_item.item().id = 1;
            ids[0] = new_item.row_id();

            new_item = item_stuff.create_item(nullptr, 1, nullptr);
            new_item.item().name = "second";
            new_item.item().description = "second description";
            new_item.item().id = 2;
            ids[1] = new_item.row_id();

            new_item = item_stuff.create_item(nullptr, 1, nullptr);
            new_item.item().name = "third";
            new_item.item().description = "third description";
            new_item.item().id = 3;
            ids[2] = new_item.row_id();

            new_item = item_stuff.create_item(nullptr, 1, nullptr);
            new_item.item().name = "fourth";
            new_item.item().description = "fourth description";
            new_item.item().id = 4;
            ids[3] = new_item.row_id();

            for (int xid = 0; xid < 4; xid++)
            {
                auto testx = ids[xid];
                for (int jid = 0; jid < 8; jid++)
                {
                    object_name test = "test detail " + std::to_string(xid) + "," + std::to_string(jid);
                    item_stuff.append_detail(testx, 1, &test);
                    auto testy = item_stuff[testx];
                    auto& det = testy.detail(jid);
                    if (det != test) {
                        std::cout << " test A didn't match " << xid << ", " << jid << " " << det << " vs " << test << std::endl;
                        r = false;
                    }
                }
            }

            for (int xid = 0; xid < 4; xid++)
            {
                auto testx = ids[xid];
                auto tc = item_stuff[testx];
                for (int jid = 0; jid < tc.size(); jid++)
                {
                    auto& det = tc.detail(jid);
                    object_name test = "test detail " + std::to_string(xid) + "," + std::to_string(jid);
                    
                    if (det != test) {
                        std::cout << " test B didn't match " << xid << ", " << jid << " " << det << " vs " << test << std::endl;
                        r = false;
                    }
                }
            }

            return r;
        }

    }
}


