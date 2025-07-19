#ifdef CORONA_BUILD
#include "coronaserver.hpp"

namespace corona::apps::revolution
{   
    void get_json(json& _dest, object_reference_type& _src)
    {
        _dest.put_member("class_name", _src.class_name);
        _dest.put_member_i64("object_id", _src.object_id);
    }

    void put_json(object_reference_type& _dest, json& _src)
    {
        _dest.class_name = _src["class_name"];
        _dest.object_id = (int64_t)_src["object_id"];
    }

    class base_object
    {
    public:
        std::string class_name;
        int64_t object_id;
        date_time created;
        std::string created_by;
        date_time updated;
        std::string updated_by;

        virtual void put_json(json& _src)
        {
            class_name = (int64_t)_src["class_name"];
            object_id = (int64_t)_src["object_id"];
            created = (date_time)_src["created"];
            created_by = (std::string)_src["created_by"];
            updated = (date_time)_src["updated"];
            updated_by = (std::string)_src["updated_by"];
        }

        virtual void get_json(json& _dest)
        {
            _dest.put_member_i64("object_id", object_id);
            _dest.put_member("class_name", class_name);
            _dest.put_member("created", created);
            _dest.put_member("created_by", created_by);
            _dest.put_member("updated", updated);
            _dest.put_member("updated", updated_by);
        }
    };


    class actor : public base_object
    {
    public:
        std::string name;
        std::string type;
        std::string state;

        double quantity = 0;

        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        double dx = 0.0;
        double dy = 0.0;
        double dz = 0.0;
        double w = 0.0;
        double h = 0.0;
        double ax = 0.0;
        double ay = 0.0;
        double az = 0.0;
        int64_t parent = 0;

        std::vector<std::shared_ptr<actor>> children;
        std::vector<object_reference_type> selection;

        virtual void put_json(json& _src) override
        {
            base_object::put_json(_src);

            name = _src["name"];
            type = _src["type"];
            state = _src["state"];
            quantity = _src["quantity"];
            parent = (int64_t)_src["parent"];

            x = (double)_src["x"];
            y = (double)_src["y"];
            z = (double)_src["z"];

            dx = (double)_src["dx"];
            dy = (double)_src["dy"];
            dz = (double)_src["dz"];

            ax = (double)_src["ax"];
            ay = (double)_src["ay"];
            az = (double)_src["az"];

            json jchildren = _src["children"];
            if (jchildren.array()) {
                children.clear();
                for (json jchild : jchildren)
                {
                    std::shared_ptr<actor> sactor = std::make_shared<actor>();
                    sactor->put_json(jchild);
                    children.push_back(sactor);
                }
            }

            json jselection = _src["selection"];
            if (jselection.array()) {
                selection.clear();
                for (json jselected : jselection)
                {
                    object_reference_type xsort;
                    corona::apps::revolution::put_json(xsort, jselected);
                    selection.push_back(xsort);
                }
            }
        }

        virtual void get_json(json& _dest) override
        {
            base_object::get_json(_dest);

            json_parser jp; 

			_dest.put_member("name", name);
			_dest.put_member("type", type);
			_dest.put_member("state", state);   

            _dest.put_member("x", x);
			_dest.put_member("y", y);
			_dest.put_member("z", z);

            _dest.put_member("dx", dx);
            _dest.put_member("dy", dy);
            _dest.put_member("dz", dz);
            _dest.put_member("quantity", quantity);
            _dest.put_member("parent", parent);

            json jchildren = jp.create_array();
            for (auto jc : children) {
                json jchild = jp.create_object();
                jc->get_json(jchild);
                jchildren.push_back(jchild);
            }
            _dest.put_member("children", jchildren);

            json jselection = jp.create_array();
            for (auto& jc : selection) {
                json jchild = jp.create_object();
                corona::apps::revolution::get_json(jchild, jc);
                jselection.push_back(jchild);
            }
            _dest.put_member("selections", jselection);
        }
    };

    class board : public base_object
    {
    public:

        std::string name;
        std::string type;
        std::string state;

        std::map<int64_t, std::shared_ptr<actor>> actors;

        virtual void put_json(json& _src) override
        {
            base_object::put_json(_src);

            actors.clear();

			json actors_json = _src["actors"];
            if (actors_json.array()) {
                for (auto actorj : actors_json)
                {
                    std::shared_ptr<actor> act = std::make_shared<actor>();
                    act->put_json(actorj);
                    actors.insert_or_assign(act->object_id, act);
                }
            }

            name = _src["name"];
            type = _src["type"];
            state = _src["state"];
        }

        virtual void get_json(json& _dest) override
        {
            base_object::get_json(_dest);
            json_parser jp;
			json jactors_json = jp.create_array();

            for (const auto& act : actors) {
                json jactor = jp.create_object();
				act.second->get_json(jactor);
				jactors_json.push_back(jactor);
            }
            _dest.put_member("actors", jactors_json);
            _dest.put_member("name", name);
            _dest.put_member("type", type);
            _dest.put_member("state", state);
        }
    };

    class recipe_ingredient : public base_object
    {
    public:
        std::string consume_class_name;
        double      amount_used;

        virtual void put_json(json& _src)
        {
            consume_class_name = (date_time)_src["create_class_name"];
            amount_used = (double)_src["amount_used"];
        }

        virtual void get_json(json& _dest)
        {
            _dest.put_member("consume_class_name", consume_class_name);
            _dest.put_member("amount_used", amount_used);
        }
    };

    class recipe_dish : public base_object
    {
    public:
        std::string create_class_name;
        double      amount_made;

        virtual void put_json(json& _src)
        {
            create_class_name = _src["create_class_name"];
            amount_made = (double)_src["amount_used"];
        }

        virtual void get_json(json& _dest)
        {
            _dest.put_member("create_class_name", create_class_name);
            _dest.put_member("amount_made", amount_made);
        }
    };

    class recipe : public base_object
    {
    public:
        std::string name;
        std::string description;
        std::vector<std::shared_ptr<recipe_ingredient>> ingredients;
        std::vector<std::shared_ptr<recipe_dish>> dishes;

        virtual void put_json(json& _src)
        {
            json_parser jp;
            ingredients.clear();
            dishes.clear();
            json jingredients = _src["ingredients"];
            if (jingredients.array()) {
                for (auto jingredient : jingredients)
                {
                    std::shared_ptr<recipe_ingredient> ingredient = std::make_shared<recipe_ingredient>();
                    ingredient->put_json(jingredient);
                    ingredients.push_back(ingredient);
                }
            }
            json jdishes = _src["dishes"];
            if (jdishes.array()) {
                for (auto jdish : jdishes)
                {
                    std::shared_ptr<recipe_dish> dish = std::make_shared<recipe_dish>();
                    dish->put_json(jdish);
                    dishes.push_back(dish);
                }
            }
        }

        virtual void get_json(json& _dest)
        {
            json_parser jp;
            json jingredients = jp.create_array();
            json jdishes = jp.create_array();
            for (auto& ingredient : ingredients)
            {
                json jingredient = jp.create_object();
                ingredient->get_json(jingredient);
                jingredients.push_back(jingredient);
            }
            for (auto& dish: dishes)
            {
                json jdish = jp.create_object();
                dish->get_json(jdish);
                jdishes.push_back(jdish);
            }
            _dest.put_member("ingredients", jingredients);
            _dest.put_member("dishes", jdishes);
        }
    };

    class game : public base_object
    {
    public:
        std::map<std::string, std::shared_ptr<board>> boards;
        std::map<std::string, std::shared_ptr<recipe>> recipes;

        void put_json(json& _src)
        {
            base_object::put_json(_src);

            boards.clear();
            json boards_json = _src["boards"];
            if (boards_json.array())
            {
                for (auto boardj : boards_json)
                {
                    std::shared_ptr<board> b = std::make_shared<board>();
                    b->put_json(boardj);
                    boards.insert_or_assign(b->name, b);
                }
            }

            recipes.clear();
            json recipes_json = _src["recipes"];
            if (recipes_json.array())
            {
                for (auto recipesj : recipes_json)
                {
                    std::shared_ptr<recipe> b = std::make_shared<recipe>();
                    b->put_json(recipesj);
                    recipes.insert_or_assign(b->name, b);
                }
            }

        }

        void get_json(json& _dest)
        {
            base_object::get_json(_dest);
            json_parser jp;
            json jboards_json = jp.create_array();
            for (auto& board_pair : boards)
            {
                json jboard = jp.create_object();
                board_pair.second->get_json(jboard);
                jboards_json.push_back(jboard);
            }
            _dest.put_member("boards", jboards_json);

            json jrecipes_json = jp.create_array();
            for (auto& recipe_pair: recipes)
            {
                json jboard = jp.create_object();
                recipe_pair.second->get_json(jboard);
                jrecipes_json.push_back(jboard);
            }
            _dest.put_member("recipes", jrecipes_json);

        }
    };

    class inventory_count
    {
    public:
        int64_t object_id;
        double quantity;
        std::shared_ptr<actor> actor;
    };

    class inventory_class {
    public:
        std::string class_name;
        double quantity;
        std::vector<inventory_count> items;
    };

    class inventory_transaction {
    public:
        std::string class_name;
        std::shared_ptr<actor> actor;
        double quantity;
    };

    class inventory
    {
        std::map<std::string, std::shared_ptr<inventory_class>> totals;

        std::shared_ptr<inventory_class> check_ingredient(std::shared_ptr<recipe_ingredient>& _ingredient)
        {
            std::shared_ptr<inventory_class> total;

            std::string class_name = _ingredient->consume_class_name;
            double quantity = _ingredient->amount_used;

            if (totals.find(class_name) == std::end(totals)) {
                return total;
            }
            else
            {
                auto& total = totals[class_name];
                total->quantity += quantity;

                if (total->quantity >= quantity) {
                    return total; // enough inventory
                }
            }
            return total;
        }

        bool use_ingredient(std::vector<inventory_transaction>& _transactions, std::shared_ptr<recipe_ingredient>&_ingredient)
        {
            std::string class_name = _ingredient->consume_class_name;
            double quantity = _ingredient->amount_used;

            if (totals.find(class_name) == std::end(totals)) {
                return false;
            }
            else
            {
                auto& total = totals[class_name];
                total->quantity -= quantity;

                for (auto& _item : total->items)
                {
                    if (_item.quantity >= quantity) {
                        quantity = 0;
                        _item.quantity -= quantity;

                        inventory_transaction transaction;
                        transaction.actor = _item.actor;
                        transaction.class_name = _item.actor->class_name;
                        transaction.quantity = -quantity;
                        _transactions.push_back(transaction);
                        break;
                    }
                    else if (_item.quantity < quantity and _item.quantity > 0.0) {
                        quantity -= _item.quantity;
                        _item.quantity = 0.0; // used up this item

                        inventory_transaction transaction;
                        transaction.actor = _item.actor;
                        transaction.class_name = _item.actor->class_name;
                        transaction.quantity = quantity;
                        _transactions.push_back(transaction);
                    }
                }
                if (quantity > 0) {
                    return false;
                }
            }

            return true;
        }

    public:

        bool use(std::vector<inventory_transaction>& _transactions, std::shared_ptr<recipe>& _ingredient)
        {
            for (auto& ingredient : _ingredient->ingredients) {
                if (not check_ingredient(ingredient)) {
                    return false; // not enough inventory
                }
            }
            for (auto& ingredient : _ingredient->ingredients) {
                use_ingredient(_transactions, ingredient);
            }
            return true;
        }

        void add(std::shared_ptr<actor> _actor)
        {
            std::string class_name = _actor->class_name;
            int64_t object_id = _actor->object_id;
            double quantity = _actor->quantity;

            if (totals.find(class_name) == std::end(totals)) {
                std::shared_ptr<inventory_class> total = std::make_shared<inventory_class>();
                total->class_name = class_name;
                total->quantity = quantity;

                inventory_count ict;
                ict.object_id = object_id;
                ict.quantity = quantity;
                ict.actor = _actor;
                total->items.push_back(ict);

                totals[class_name] = total;
            }
            else 
            {
                auto& total = totals[class_name];
                total->quantity += quantity;

                inventory_count ict;
                ict.object_id = object_id;
                ict.quantity = quantity;
                ict.actor = _actor;
                total->items.push_back(ict);
            }
        }

    };

    class revolution_simulation : public corona_simulation_interface
    {


        template <typename object_type> std::shared_ptr<object_type> from_object(json& _object)
        {
            std::shared_ptr<object_type> result = nullptr;

            result = std::make_shared<object_type>();

            if (_object.object())
            {
                result->put_json(_object);
            }

            return result;
        }

        template <typename object_type> std::shared_ptr<object_type> get_object(std::string _class_name, int64_t _object_id, bool _include_children)
        {
            std::shared_ptr<object_type> result = nullptr;

            json response = service->get_object(_class_name, _object_id, _include_children);

            if ((bool)response[success_field] == false)
            {
                // Handle error case
                return nullptr;
            }

            result = std::make_shared<object_type>();

            json result_data = response["data"];
            if (result_data.object())
            {
                result->put_json(result_data);
            }

            return result;
        }

        template <typename object_type> void put_object(std::shared_ptr<object_type> _obj)
        {
            json_parser jp;
            json obj = jp.create_object();

            _obj->get_json(obj);

            json response =service->put_object(obj);

            if (response[success_field])
            {
                json result_data = response["data"];

                if (result_data.object())
                {
                    _obj->put_json(result_data);
                }
            }
        }

        template <typename object_type> std::shared_ptr<object_type> create_object(std::string _class_name)
        {
            json_parser jp;
            std::shared_ptr<object_type> result = nullptr;

            json response = service->create_object(_class_name);

            if (response[success_field])
            {
                json result_data = response["data"];
                if (result_data.object())
                {
                    result = std::make_shared<object_type>();
                    result->put_json(result_data);
                }
            }
            return result;
        }

        std::string get_command_user(json _token)
        {
            ;
        }

    public:

        double selection_distance = 5.0;

        std::shared_ptr<inventory> get_selection(std::shared_ptr<actor> _actor)
        {
            std::shared_ptr<inventory> inv = std::make_shared<inventory>();
            for (auto sel : _actor->selection) {
                auto target = get_actor(sel, false);
                if (target) {
                    inv->add(target);
                }
            }
            return inv;
        }

        std::shared_ptr<board> get_board(object_reference_type& _ort, bool _recursive)
        {
            // This is a placeholder for the actual implementation
            // that would retrieve a board by its ID.
            return get_object<board>("board", _ort.object_id, _recursive);
        }

        std::shared_ptr<game> get_game(object_reference_type& _ort, bool _recursive)
        {
            // This is a placeholder for the actual implementation 
            // that would retrieve a board by its ID.
            return get_object<game>("game", _ort.object_id, _recursive);
        }

        std::shared_ptr<actor> get_actor(object_reference_type& _ort, bool _recursive)
        {
            // This is a placeholder for the actual implementation 
            // that would retrieve a board by its ID.
            return get_object<actor>("actor", _ort.object_id, _recursive);
        }

        std::shared_ptr<board> get_board(json& _src_command, bool _recursive)
        {
            std::shared_ptr<board> ret;
            // This is a placeholder for the actual implementation
            // that would retrieve a board by its ID.
            json ref = _src_command["board"];
            if (ref.reference()) {
                ref.reference_impl()->value.class_name;
                ref.reference_impl()->value.object_id;
                ret = get_board(_src_command, _recursive);
            }
            return ret;
        }

        std::shared_ptr<game> get_game(json& _src_command, bool _recursive)
        {
            std::shared_ptr<game> ret;
            // This is a placeholder for the actual implementation
            // that would retrieve a board by its ID.
            json ref = _src_command["game"];
            if (ref.reference()) {
                ref.reference_impl()->value.class_name;
                ref.reference_impl()->value.object_id;
                ret = get_game(_src_command, true);
            }
            return ret;
        }

        std::shared_ptr<actor> get_actor(json& _src_command, bool _recursive)
        {
            // This is a placeholder for the actual implementation 
            // that would retrieve a board by its ID.
            std::shared_ptr<actor> ret;
            // This is a placeholder for the actual implementation
            // that would retrieve a board by its ID.
            json ref = _src_command["actor"];
            if (ref.reference()) {
                ref.reference_impl()->value.class_name;
                ref.reference_impl()->value.object_id;
                ret = get_actor(_src_command, true);
            }
            return ret;
        }

        std::shared_ptr<actor> put_actor(std::shared_ptr<actor> _actor)
        {
            put_object(_actor);
            return _actor;
        }

        std::shared_ptr<board> put_board(std::shared_ptr<board> _actor)
        {
            put_object(_actor);
            return _actor;
        }

        std::shared_ptr<game> put_game(std::shared_ptr<game> _actor)
        {
            put_object(_actor);
            return _actor;
        }

        void clear_selection(json& _command)
        {
            auto pactor = get_actor(_command, true);
            if (pactor) {
                pactor->selection.clear();
                put_actor(pactor);
            }
        }

        void extend_selection(json& _command)
        {
            auto pactor = get_actor(_command, true);
            if (pactor) {
                json_parser jp;
                json add_actors = _command["add_actors"];
                if (add_actors.array()) {
                    for (json jactor : add_actors)
                    {
                        object_reference_type xsort;
                        corona::apps::revolution::put_json(xsort, jactor);
                        auto target = get_actor(jactor, false);
                        if (target)
                        {
                            double distance = point_math::distance({ target->x, target->y, target->z }, { target->x, target->y, target->z });
                            if (distance < selection_distance)
                            {
                                pactor->selection.push_back(xsort);
                            }
                        }
                    }
                    put_actor(pactor);
                }
            }
        }

        void compose(json& _command)
        {
            auto pactor = get_actor(_command, true);
            auto pgame = get_game(_command, true);
            auto actor_selection = get_selection(pactor);

            if (pactor and pgame) {
                std::vector<inventory_transaction> transactions;
                bool can_compose = true;
                std::string compose_class_name = _command["compose_class_name"];
                auto found_recipes = pgame->recipes.find(compose_class_name);
                if (found_recipes != std::end(pgame->recipes)) {
                    // take the things away
                    auto my_recipe = found_recipes->second;
                    actor_selection->use(transactions, my_recipe);
                    for (auto& tran : transactions)
                    {
                        if (tran.actor)
                        {
                            tran.actor->quantity = tran.quantity;
                            put_actor(tran.actor);
                        }
                    }
                    // then add the new things
                    for (auto& dish : my_recipe->dishes)
                    {
                        std::shared_ptr<actor> new_actor = create_object<actor>(dish->create_class_name);
                        if (new_actor) {
                            new_actor->quantity = dish->amount_made;
                            new_actor->x = 0;
                            new_actor->y = 0;
                            new_actor->z = 0;
                            new_actor->parent = pactor->object_id; // parent is the actor that composed it
                            put_actor(new_actor);
                        }
                    }
                }
            }
        }

        void take(json& _command)
        {
            auto pactor = get_actor(_command, true);
            auto board = get_board(_command, true);

            if (pactor and board) {

                json take_actors = _command["take_actors"];
                if (take_actors.array()) {
                    for (json jactor : take_actors)
                    {
                        object_reference_type xsort;
                        corona::apps::revolution::put_json(xsort, jactor);
                        auto target = get_actor(jactor, false);

                        // simple takeability constraint that we will refine later
                        if (target)
                        {
                            double distance = point_math::distance({ target->x, target->y, target->z }, { target->x, target->y, target->z });
                            if (distance < selection_distance)
                            {
                                target->parent = pactor->object_id; // take the actor
                                put_actor(target);
                            }
                        }
                    }
                }
            }
        }

        void drop(json& _command)
        {
            auto pactor = get_actor(_command, true);
            auto board = get_board(_command, true);

            if (pactor and board) {
                json drop_actors = _command["drop_actors"];
                if (drop_actors.array()) {
                    for (json jactor : drop_actors)
                    {
                        object_reference_type xsort;
                        corona::apps::revolution::put_json(xsort, jactor);
                        auto target = get_actor(jactor, false);

                        // simple drop constraint that we will refine later
                        if (target and target->parent == pactor->object_id)
                        {
                            target->parent = board->object_id; // put the actor
                            target->x = pactor->x; // drop it at the actor's location
                            target->y = pactor->y;
                            target->z = pactor->z;
                            put_actor(target);
                        }
                    }
                }
            }
        }

        void accelerate(json& _command)
        {
            auto pactor = get_actor(_command, true);
            auto board = get_board(_command, true);
            if (pactor) {
                if (pactor and board) {
                    json accelerate_actors = _command["accelerate_actors"];
                    if (accelerate_actors.array()) {
                        for (json jactor : accelerate_actors)
                        {
                            object_reference_type xsort;
                            corona::apps::revolution::put_json(xsort, jactor);
                            auto target = get_actor(jactor, false);

                            // simple constraint that we will refine later
                            if (target and target->parent == pactor->object_id)
                            {
                                target->ax = _command["ax"];
                                target->ay = _command["ay"];
                                target->az = _command["az"];
                                put_actor(target);
                            }
                        }
                    }
                }
            }

        }

        void activate(json& _command)
        {
            auto pactor = get_actor(_command, true);
            if (pactor) {
            }
        }

        void navigate(json& _command)
        {
            auto pactor = get_actor(_command, true);
            if (pactor) {
                pactor->selection.clear();
                put_actor(pactor);
            }
        }

        void join_game(json& _command)
        {
            auto pactor = get_actor(_command, true);
            if (pactor) {

            }
        }                           

        void exit_game(json& _command)
        {
            auto pactor = get_actor(_command, true);
            if (pactor) {
                pactor->selection.clear();
                put_actor(pactor);
            }
        }

        virtual void on_frame(json& _commands)
        {
            if (_commands.object()) 
            {
                execute(_commands);
            }
            else if (_commands.array()) 
            {
                for (auto command : _commands)
                {
                    execute(command);
                }
            }
            json games = service->get_data("game");
            if (games.array()) {
                for (auto jgame : games) {
                    std::shared_ptr<game> pgame = std::make_shared<game>();
                    pgame->put_json(jgame);
                    for (auto& board_pair : pgame->boards) {
                        auto& board = board_pair.second;
                        for (auto& actor_pair : board->actors) {
                            auto& actor = actor_pair.second;
                            actor->dx += actor->ax;
                            actor->dy += actor->ay;
                            actor->dz += actor->az;
                            actor->x += actor->dx;
                            actor->y += actor->dy; 
                            actor->z += actor->dz;
                            actor->ax = 0.0;
                            actor->ay = 0.0;
                            actor->az = 0.0;
                            put_actor(actor);
                        }
                    }
                    put_game(pgame);
                }
            }
        }

        virtual void execute(json& _command)
        {
            std::string class_name = _command["class_name"];
            int64_t object_id = (int64_t)_command["object_id"];

            try {

                if (class_name == "select_clear_command")
                {
                    clear_selection(_command);
                }
                else if (class_name == "select_extend_command")
                {
                    extend_selection(_command);;
                }
                else if (class_name == "compose_command")
                {
                    compose(_command);
                }
                else if (class_name == "take_command")
                {
                    take(_command);
                }
                else if (class_name == "drop_command")
                {
                    drop(_command);
                }
                else if (class_name == "accelerate_command")
                {
                    accelerate(_command);
                }
                else if (class_name == "activate_command")
                {
                    activate(_command);
                }
                else if (class_name == "navigate_command")
                {
                    navigate(_command);
                }
                else if (class_name == "join_game_command")
                {
                    join_game(_command);
                }
                else if (class_name == "exit_game_command")
                {
                    exit_game(_command);
                }

                service->delete_object(class_name, object_id);
            }
            catch (std::exception& e)
            {
                service->log_exception(e, __FILE__, __LINE__);
            }
        }
    };

}



int main(int argc, char* argv[])
{
    std::shared_ptr<corona::apps::revolution::revolution_simulation> simulation = std::make_shared<corona::apps::revolution::revolution_simulation>();
    return CoronaMain(simulation, argc, argv);
}

#endif

