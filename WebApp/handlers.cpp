#include "handlers.h"

#include <vector>

#include "rendering.h"

// register an orm mapping (to convert the db query results into
// json objects).
// the db query results contain several rows, each has a number of
// fields. the order of `make_db_field<Type[i]>(name[i])` in the
// initializer menu corresponds to these fields (`Type[0]` and
// `name[0]` correspond to field[0], `Type[1]` and `name[1]`
// correspond to field[1], ...). `Type[i]` is the type you want
// to convert the field value to, and `name[i]` is the identifier
// with which you want to store the field in the json object, so
// if the returned json object is `obj`, `obj[name[i]]` will have
// the type of `Type[i]` and store the value of field[i].
bserv::db_relation_to_object orm_user{
	bserv::make_db_field<int>("id"),
	bserv::make_db_field<std::string>("username"),
	bserv::make_db_field<std::string>("password"),
	bserv::make_db_field<bool>("is_superuser"),
	bserv::make_db_field<std::string>("first_name"),
	bserv::make_db_field<std::string>("last_name"),
	bserv::make_db_field<std::string>("email"),
	bserv::make_db_field<bool>("is_active")
};

bserv::db_relation_to_object orm_list{
	bserv::make_db_field<int>("id"),
	bserv::make_db_field<std::string>("dishname"),
	bserv::make_db_field<int>("price"),
	bserv::make_db_field<std::string>("spiciness"),
	bserv::make_db_field<std::string>("cname"),

};

bserv::db_relation_to_object orm_spiciness{
	bserv::make_db_field<std::string>("spiciness"),
};

bserv::db_relation_to_object orm_cuisine{
	bserv::make_db_field<std::string>("cname"),
};
bserv::db_relation_to_object orm_price{
	bserv::make_db_field<std::string>("total_price"),
};

bserv::db_relation_to_object orm_ordered{
	bserv::make_db_field<std::string>("dname"),
	bserv::make_db_field<std::string>("cname"),
	bserv::make_db_field<std::string>("uname"),
	bserv::make_db_field<int>("ocount"),
	bserv::make_db_field<bool>("is_favorite"),
};
bserv::db_relation_to_object orm_more{
	bserv::make_db_field<std::string>("cname"),
	bserv::make_db_field<std::string>("area"),
	bserv::make_db_field<std::string>("message"),
	bserv::make_db_field<std::string>("sample"),
};

bserv::db_relation_to_object orm_dish{
	bserv::make_db_field<std::string>("dishname"),

};

bserv::db_relation_to_object orm_orderin{
	bserv::make_db_field<std::string>("dname"),
	bserv::make_db_field<std::string>("uname"),
	bserv::make_db_field<int>("ocount"),
	bserv::make_db_field<bool>("is_favorite"),

};

std::optional<boost::json::object> get_user(
	bserv::db_transaction& tx,
	const boost::json::string& username) {
	bserv::db_result r = tx.exec(
		"select * from auth_user where username = ?", username);
	lginfo << r.query();
	return orm_user.convert_to_optional(r);
}

std::optional<boost::json::object> get_list(
	bserv::db_transaction& tx,
	const boost::json::string& dishname) {
	bserv::db_result r = tx.exec(
		"select * from dish where dishname = ?", dishname);
	lginfo << r.query();
	return orm_list.convert_to_optional(r);
}

std::optional<boost::json::object> get_cuisine(
	bserv::db_transaction& tx,
	const boost::json::string& cname) {
	bserv::db_result r = tx.exec(
		"select cname from cuisines where cname = ?", cname);
	lginfo << r.query(); 
	return orm_cuisine.convert_to_optional(r);
}

std::optional<boost::json::object> get_orderin(
	bserv::db_transaction& tx,
	const boost::json::string& dishname,
	const boost::json::string& username) {
	bserv::db_result r = tx.exec(
		"select * from ordered where dname = ? and uname=?", dishname,username);
	lginfo << r.query();
	return orm_orderin.convert_to_optional(r);
}

std::string get_or_empty(
	boost::json::object& obj,
	const std::string& key) {
	return obj.count(key) ? obj[key].as_string().c_str() : "";
}

std::string get_or_zero(
	boost::json::object& obj,
	const std::string& key) {
	return obj.count(key) ? obj[key].as_string().c_str() : 0;
}

// if you want to manually modify the response,
// the return type should be `std::nullopt_t`,
// and the return value should be `std::nullopt`.
std::nullopt_t hello(
	bserv::response_type& response,
	std::shared_ptr<bserv::session_type> session_ptr) {
	bserv::session_type& session = *session_ptr;
	boost::json::object obj;
	if (session.count("user")) {
		// NOTE: modifications to sessions must be performed
		// BEFORE referencing objects in them. this is because
		// modifications might invalidate referenced objects.
		// in this example, "count" might be added to `session`,
		// which should be performed first.
		// then `user` can be referenced safely.
		if (!session.count("count")) {
			session["count"] = 0;
		}
		auto& user = session["user"].as_object();
		session["count"] = session["count"].as_int64() + 1;
		obj = {
			{"welcome", user["username"]},
			{"count", session["count"]}
		};
	}
	else {
		obj = { {"msg", "hello, world!"} };
	}
	// the response body is a string,
	// so the `obj` should be serialized
	response.body() = boost::json::serialize(obj);
	response.prepare_payload(); // this line is important!
	return std::nullopt;
}

// if you return a json object, the serialization
// is performed automatically.
boost::json::object user_register(
	bserv::request_type& request,
	// the json object is obtained from the request body,
	// as well as the url parameters
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	if (request.method() != boost::beast::http::verb::post) {
		throw bserv::url_not_found_exception{};
	}
	if (params.count("username") == 0) {
		return {
			{"success", false},
			{"message", "`username` is required"}
		};
	}
	if (params.count("password") == 0) {
		return {
			{"success", false},
			{"message", "`password` is required"}
		};
	}
	auto username = params["username"].as_string();
	bserv::db_transaction tx{ conn };
	auto opt_user = get_user(tx, username);
	if (opt_user.has_value()) {
		return {
			{"success", false},
			{"message", "`username` existed"}
		};
	}
	auto password = params["password"].as_string();
	bserv::db_result r = tx.exec(
		"insert into ? "
		"(?, password, is_superuser, "
		"first_name, last_name, email, is_active) values "
		"(?, ?, ?, ?, ?, ?, ?)", bserv::db_name("auth_user"),
		bserv::db_name("username"),
		username,
		bserv::utils::security::encode_password(
			password.c_str()), 
		(get_or_empty(params,"superuser")=="true")?true:false,
		get_or_empty(params, "first_name"),
		get_or_empty(params, "last_name"),
		get_or_empty(params, "email"), true);
	lginfo << r.query();
	tx.commit(); // you must manually commit changes
	return {
		{"success", true},
		{"message", "user registered"}
	};
}

boost::json::object user_delete(
	bserv::request_type& request,
	// the json object is obtained from the request body,
	// as well as the url parameters
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	if (request.method() != boost::beast::http::verb::post) {
		throw bserv::url_not_found_exception{};
	}
	if (params.count("duser") == 0) {
		return {
			{"success", false},
			{"message", "`username` is required"}
		};
	}
	if (params.count("password") == 0) {
		return {
			{"success", false},
			{"message", "`password` is required"}
		};
	}
	auto username = params["duser"].as_string();
	bserv::db_transaction tx{ conn };
	auto opt_user = get_user(tx, username);
	if (!opt_user.has_value()) {
		return {
			{"success", false},
			{"message", "`username` does not exist"}
		};
	}
	auto& user = opt_user.value();
	if (!user["is_active"].as_bool()) {
		return {
			{"success", false},
			{"message", "invalid username/password"}
		};
	}
	auto password = params["password"].as_string();
	auto encoded_password = user["password"].as_string();
	if (!bserv::utils::security::check_password(
		password.c_str(), encoded_password.c_str())) {
		return {
			{"success", false},
			{"message", "invalid username/password"}
		};
	}
	bserv::db_result r = tx.exec(
		"update ?"
		"set is_active=false "
		"where ?=? ",
		 bserv::db_name("auth_user"),
		bserv::db_name("username"),
		username);
	lginfo << r.query();
	tx.commit(); // you must manually commit changes
	return {
		{"success", true},
		{"message", "user deleted"}
	};
}

boost::json::object menu_delete(
	bserv::request_type& request,
	// the json object is obtained from the request body,
	// as well as the url parameters
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	if (request.method() != boost::beast::http::verb::post) {
		throw bserv::url_not_found_exception{};
	}
	if (params.count("colname") == 0) {
		return {
			{"success", false},
			{"message", "`username` is required"}
		};
	}

	auto dname = params["colname"].as_string();
	bserv::db_transaction tx{ conn };
	auto opt_user = get_list(tx, dname);
	if (!opt_user.has_value()) {
		return {
			{"success", false},
			{"message", "`dish` does not exist"}
		};
	}

	bserv::db_result r = tx.exec(
		"delete from ?"
		"where dname=? ",
		bserv::db_name("ordered"),
		get_or_empty(params, "colname"));

	 r = tx.exec(
		"delete from ?"
		"where dishname=? ",
		bserv::db_name("dish"),
		get_or_empty(params,"colname"));
	lginfo << r.query();
	tx.commit(); // you must manually commit changes
	return {
		{"success", true},
		{"message", "menu deleted"}
	};
}

boost::json::object menu_register(
	bserv::request_type& request,
	// the json object is obtained from the request body,
	// as well as the url parameters
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	if (request.method() != boost::beast::http::verb::post) {
		throw bserv::url_not_found_exception{};
	}
	if (params.count("dishname") == 0) {
		return {
			{"success", false},
			{"message", "`dishname` is required"}
		};
	}
	lgdebug << params["spiciness"] ;
	auto dishname = params["dishname"].as_string();

	bserv::db_transaction tx{ conn };
	auto opt_user = get_list(tx, dishname);
	if (opt_user.has_value()) {
		return {
			{"success", false},
			{"message", "`dishname` existed"}
		};
	}

	bserv::db_result r = tx.exec(
		"insert into ? "
		"(?, ?, "
		"?, ?) values "
		"(?, ?, ?, ? )", bserv::db_name("dish"),
		bserv::db_name("dishname"),
		bserv::db_name("price"),
		bserv::db_name("spiciness"),
		bserv::db_name("cname"),

		get_or_empty(params, "dishname"),
		
		get_or_zero(params, "price"),
		get_or_empty(params, "spiciness"), 
		get_or_empty(params, "cname")
		);
	lginfo << r.query();
	tx.commit(); // you must manually commit changes
	return {
		{"success", true},
		{"message", "menu added"}
	};
}
boost::json::object cuisine_register(
	bserv::request_type& request,
	// the json object is obtained from the request body,
	// as well as the url parameters
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	if (request.method() != boost::beast::http::verb::post) {
		throw bserv::url_not_found_exception{};
	}
	if (params.count("addcname") == 0) {
		return {
			{"success", false},
			{"message", "`CuisineName` is required"}
		};
	}

	lgdebug << params["spiciness"];
	auto cname = params["addcname"].as_string();

	bserv::db_transaction tx{ conn };
	auto opt_user = get_cuisine(tx, cname);
	if (opt_user.has_value()) {
		bserv::db_result r = tx.exec(
			"update ? "
			"set ?=?, ?=?, ?=?  "
			"where ?=?",
			bserv::db_name("cuisines"),
			

			bserv::db_name("message"),
			get_or_empty(params, "message"),
			bserv::db_name("sample"),
			get_or_empty(params, "sample"),
			bserv::db_name("area"),
			get_or_empty(params, "area"),
			bserv::db_name("cname"),
			get_or_empty(params, "addcname")

			
		);
	}
	else {
		bserv::db_result r = tx.exec(
			"insert into ? "
			"(?, ?, "
			"?, ?) values "
			"(?, ?, ?,? )", bserv::db_name("cuisines"),
			bserv::db_name("cname"),
			bserv::db_name("message"),
			bserv::db_name("sample"),
			bserv::db_name("area"),
			get_or_empty(params, "addcname"),

			get_or_empty(params, "message"),
			get_or_empty(params, "sample"),
			get_or_empty(params, "area")
		);
	}
	tx.commit(); // you must manually commit changes
	return {
		{"success", true},
		{"message", "cuisine registered"}
	};
}
boost::json::object menu_orderin(
	bserv::request_type& request,
	// the json object is obtained from the request body,
	// as well as the url parameters
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	if (request.method() != boost::beast::http::verb::post) {
		throw bserv::url_not_found_exception{};
	}
	if (params.count("colname") == 0) {
		return {
			{"success", false},
			{"message", "`dishname` is required"}
		};
	}
	lgdebug << params["spiciness"];
	auto dishname = params["colname"].as_string();
	auto username = params["coluser"].as_string();
	bserv::db_transaction tx{ conn };
	auto opt_user = get_orderin(tx, dishname,username);
	if (opt_user.has_value()) {
		return {
			{"success", false},
			{"message", "`dish` has been ordered"}
		};
	}
	bserv::db_result r = tx.exec(
		"insert into ? "
		"(?, ?, "
		"?, ?, ?) values "
		"(?, ?, ?, ?, ? )", bserv::db_name("ordered"),
		bserv::db_name("uname"),
		bserv::db_name("dname"),
		bserv::db_name("price"),
		bserv::db_name("ocount"),
		bserv::db_name("is_favorite"),

		get_or_empty(params, "coluser"),

		get_or_empty(params, "colname"),

		get_or_empty(params, "colprice"),
		1,
		false);


	lginfo << r.query();
	tx.commit(); // you must manually commit changes
	return {
		{"success", true},
		{"message", "dish ordered"}
	};
}

boost::json::object favor_set(
	bserv::request_type& request,
	// the json object is obtained from the request body,
	// as well as the url parameters
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	if (request.method() != boost::beast::http::verb::post) {
		throw bserv::url_not_found_exception{};
	}
	if (params.count("setname") == 0) {
		return {
			{"success", false},
			{"message", "`dishname` is required"}
		};
	}
	lgdebug << params["spiciness"];
	auto dishname = params["setname"].as_string();
	auto username = params["setuser"].as_string();
	bserv::db_transaction tx{ conn };

	bserv::db_result r = tx.exec(
		"update  ? "
		"set is_favorite=not is_favorite "
		"where dname=? and uname=? ",
		bserv::db_name("ordered"),


		get_or_empty(params, "setname"),

		get_or_empty(params, "setuser")
		
		);


	lginfo << r.query();
	tx.commit(); // you must manually commit changes
	return {
		{"success", true},
		{"message", "Favor changed"}
	};
}

boost::json::object play_set(
	bserv::request_type& request,
	// the json object is obtained from the request body,
	// as well as the url parameters
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	if (request.method() != boost::beast::http::verb::post) {
		throw bserv::url_not_found_exception{};
	}
	if (params.count("setname") == 0) {
		return {
			{"success", false},
			{"message", "`dishname` is required"}
		};
	}
	lgdebug << params["spiciness"];
	auto dishname = params["setname"].as_string();
	auto username = params["setuser"].as_string();
	bserv::db_transaction tx{ conn };

	bserv::db_result r = tx.exec(
		"update  ? "
		"set ocount =ocount+1 "
		"where dname=? and uname=? ",
		bserv::db_name("ordered"),

		get_or_empty(params, "setname"),

		get_or_empty(params, "setuser")

	);


	lginfo << r.query();
	tx.commit(); // you must manually commit changes
	return {
		{"success", true},
		{"message", "Added"}
	};
}

boost::json::object clear_set(
	bserv::request_type& request,
	// the json object is obtained from the request body,
	// as well as the url parameters
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	if (request.method() != boost::beast::http::verb::post) {
		throw bserv::url_not_found_exception{};
	}
	if (params.count("setname") == 0) {
		return {
			{"success", false},
			{"message", "`dishname` is required"}
		};
	}
	lgdebug << params["spiciness"];
	auto dishname = params["setname"].as_string();
	auto username = params["setuser"].as_string();
	bserv::db_transaction tx{ conn };

	bserv::db_result r = tx.exec(
		"update  ? "
		"set ocount =1 "
		"where dname=? and uname=? ",
		bserv::db_name("ordered"),

		get_or_empty(params, "setname"),

		get_or_empty(params, "setuser")

	);


	lginfo << r.query();
	tx.commit(); // you must manually commit changes
	return {
		{"success", true},
		{"message", "Reset Count"}
	};
}

boost::json::object delete_set(
	bserv::request_type& request,
	// the json object is obtained from the request body,
	// as well as the url parameters
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	if (request.method() != boost::beast::http::verb::post) {
		throw bserv::url_not_found_exception{};
	}
	if (params.count("setname") == 0) {
		return {
			{"success", false},
			{"message", "`dishname` is required"}
		};
	}
	lgdebug << params["spiciness"];
	auto dishname = params["setname"].as_string();
	auto username = params["setuser"].as_string();
	bserv::db_transaction tx{ conn };

	bserv::db_result r = tx.exec(
		"delete from  ? "
		"where dname=? and uname=? ",
		bserv::db_name("ordered"),

		get_or_empty(params, "setname"),

		get_or_empty(params, "setuser")

	);


	lginfo << r.query();
	tx.commit(); // you must manually commit changes
	return {
		{"success", true},
		{"message", "Cancel Succesfully"}
	};
}

boost::json::object td_favor_set(
	bserv::request_type& request,
	// the json object is obtained from the request body,
	// as well as the url parameters
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	if (request.method() != boost::beast::http::verb::post) {
		throw bserv::url_not_found_exception{};
	}
	if (params.count("setname") == 0) {
		return {
			{"success", false},
			{"message", "`dishname` is required"}
		};
	}
	lgdebug << params["spiciness"];
	auto dishname = params["setname"].as_string();
	auto username = params["setuser"].as_string();
	bserv::db_transaction tx{ conn };

	bserv::db_result r = tx.exec(
		"update  ? "
		"set is_favorite=not is_favorite "
		"where dname=? and uname=? ",
		bserv::db_name("ordered"),

		get_or_empty(params, "setname"),

		get_or_empty(params, "setuser")
		
		);


	lginfo << r.query();
	tx.commit(); // you must manually commit changes
	return {
		{"success", true},
		{"message", "Favor changed"}
	};
}

boost::json::object td_play_set(
	bserv::request_type& request,
	// the json object is obtained from the request body,
	// as well as the url parameters
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	if (request.method() != boost::beast::http::verb::post) {
		throw bserv::url_not_found_exception{};
	}
	if (params.count("setname") == 0) {
		return {
			{"success", false},
			{"message", "`dishname` is required"}
		};
	}
	lgdebug << params["spiciness"];
	auto dishname = params["setname"].as_string();
	auto username = params["setuser"].as_string();
	bserv::db_transaction tx{ conn };

	bserv::db_result r = tx.exec(
		"update  ? "
		"set ocount =ocount+1 "
		"where dname=? and uname=? ",
		bserv::db_name("ordered"),

		get_or_empty(params, "setname"),

		get_or_empty(params, "setuser")

	);


	lginfo << r.query();
	tx.commit(); // you must manually commit changes
	return {
		{"success", true},
		{"message", "Added"}
	};
}

boost::json::object td_clear_set(
	bserv::request_type& request,
	// the json object is obtained from the request body,
	// as well as the url parameters
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	if (request.method() != boost::beast::http::verb::post) {
		throw bserv::url_not_found_exception{};
	}
	if (params.count("setname") == 0) {
		return {
			{"success", false},
			{"message", "`dishname` is required"}
		};
	}
	lgdebug << params["spiciness"];
	auto dishname = params["setname"].as_string();
	auto username = params["setuser"].as_string();
	bserv::db_transaction tx{ conn };

	bserv::db_result r = tx.exec(
		"update  ? "
		"set ocount =1 "
		"where dname=? and uname=? ",
		bserv::db_name("ordered"),

		get_or_empty(params, "setname"),

		get_or_empty(params, "setuser")

	);


	lginfo << r.query();
	tx.commit(); // you must manually commit changes
	return {
		{"success", true},
		{"message", "Reset Count"}
	};
}

boost::json::object td_delete_set(
	bserv::request_type& request,
	// the json object is obtained from the request body,
	// as well as the url parameters
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	if (request.method() != boost::beast::http::verb::post) {
		throw bserv::url_not_found_exception{};
	}
	if (params.count("setname") == 0) {
		return {
			{"success", false},
			{"message", "`dishname` is required"}
		};
	}
	lgdebug << params["spiciness"];
	auto dishname = params["setname"].as_string();
	auto username = params["setuser"].as_string();
	bserv::db_transaction tx{ conn };

	bserv::db_result r = tx.exec(
		"delete from  ? "
		"where dname=? and uname=? ",
		bserv::db_name("ordered"),

		get_or_empty(params, "setname"),

		get_or_empty(params, "setuser")

	);


	lginfo << r.query();
	tx.commit(); // you must manually commit changes
	return {
		{"success", true},
		{"message", "Done Succesfully"}
	};
}

boost::json::object user_login(
	bserv::request_type& request,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	if (request.method() != boost::beast::http::verb::post) {
		throw bserv::url_not_found_exception{};
	}
	if (params.count("username") == 0) {
		return {
			{"success", false},
			{"message", "`username` is required"}
		};
	}
	if (params.count("password") == 0) {
		return {
			{"success", false},
			{"message", "`password` is required"}
		};
	}
	auto username = params["username"].as_string();
	bserv::db_transaction tx{ conn };
	auto opt_user = get_user(tx, username);
	if (!opt_user.has_value()) {
		return {
			{"success", false},
			{"message", "invalid username/password"}
		};
	}
	auto& user = opt_user.value();
	if (!user["is_active"].as_bool()) {
		return {
			{"success", false},
			{"message", "invalid username/password"}
		};
	}
	auto password = params["password"].as_string();
	auto encoded_password = user["password"].as_string();
	if (!bserv::utils::security::check_password(
		password.c_str(), encoded_password.c_str())) {
		return {
			{"success", false},
			{"message", "invalid username/password"}
		};
	}
	bserv::session_type& session = *session_ptr;
	session["user"] = user;
	return {
		{"success", true},
		{"message", "login successfully"}
	};
}

boost::json::object find_user(
	std::shared_ptr<bserv::db_connection> conn,
	const std::string& username) {
	bserv::db_transaction tx{ conn };
	auto user = get_user(tx, username.c_str());
	if (!user.has_value()) {
		return {
			{"success", false},
			{"message", "requested user does not exist"}
		};
	}
	user.value().erase("id");
	user.value().erase("password");
	return {
		{"success", true},
		{"user", user.value()}
	};
}

boost::json::object user_logout(
	std::shared_ptr<bserv::session_type> session_ptr) {
	bserv::session_type& session = *session_ptr;
	if (session.count("user")) {
		session.erase("user");
	}
	return {
		{"success", true},
		{"message", "logout successfully"}
	};
}

boost::json::object send_request(
	std::shared_ptr<bserv::session_type> session,
	std::shared_ptr<bserv::http_client> client_ptr,
	boost::json::object&& params) {
	// post for response:
	// auto res = client_ptr->post(
	//     "localhost", "8080", "/echo", {{"msg", "request"}}
	// );
	// return {{"response", boost::json::parse(res.body())}};
	// -------------------------------------------------------
	// - if it takes longer than 30 seconds (by default) to
	// - get the response, this will raise a read timeout
	// -------------------------------------------------------
	// post for json response (json value, rather than json
	// object, is returned):
	auto obj = client_ptr->post_for_value(
		"localhost", "8080", "/echo", { {"request", params} }
	);
	if (session->count("cnt") == 0) {
		(*session)["cnt"] = 0;
	}
	(*session)["cnt"] = (*session)["cnt"].as_int64() + 1;
	return { {"response", obj}, {"cnt", (*session)["cnt"]} };
}

boost::json::object echo(
	boost::json::object&& params) {
	return { {"echo", params} };
}

// websocket
std::nullopt_t ws_echo(
	std::shared_ptr<bserv::session_type> session,
	std::shared_ptr<bserv::websocket_server> ws_server) {
	ws_server->write_json((*session)["cnt"]);
	while (true) {
		try {
			std::string data = ws_server->read();
			ws_server->write(data);
		}
		catch (bserv::websocket_closed&) {
			break;
		}
	}
	return std::nullopt;
}


std::nullopt_t serve_static_files(
	bserv::response_type& response,
	const std::string& path) {
	return serve(response, path);
}


std::nullopt_t index(
	const std::string& template_path,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	boost::json::object& context) {
	bserv::session_type& session = *session_ptr;
	if (session.contains("user")) {
		context["user"] = session["user"];
	}
	return render(response, template_path, context);
}


std::nullopt_t index_page(
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response) {
	boost::json::object context;
	return index("index.html", session_ptr, response, context);
}

std::nullopt_t form_login(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	lgdebug << params << std::endl;
	auto context = user_login(request, std::move(params), conn, session_ptr);
	lginfo << "login: " << context << std::endl;
	return index("index.html", session_ptr, response, context);
}

std::nullopt_t form_logout(
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response) {
	auto context = user_logout(session_ptr);
	lginfo << "logout: " << context << std::endl;
	return index("index.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_users(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	int page_id,
	boost::json::object&& context) {
	lgdebug << "view users: " << page_id << std::endl;
	bserv::db_transaction tx{ conn };
	bserv::db_result db_res = tx.exec("select count(*) from auth_user;");
	lginfo << db_res.query();
	std::size_t total_users = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total users: " << total_users << std::endl;
	int total_pages = (int)total_users / 10;
	if (total_users % 10 != 0) ++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;
	db_res = tx.exec("select * from auth_user where is_active=true limit 10 offset ?;", (page_id - 1) * 10);
	lginfo << db_res.query();
	auto users = orm_user.convert_to_vector(db_res);
	boost::json::array json_users;
	for (auto& user : users) {
		json_users.push_back(user);
	}
	boost::json::object pagination;
	if (total_pages != 0) {
		pagination["total"] = total_pages;
		if (page_id > 1) {
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages) {
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2) {
			pagination["left_ellipsis"] = true;
		}
		else {
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1) {
			pagination["right_ellipsis"] = true;
		}
		else {
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i) {
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i) {
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["users"] = json_users;
	return index("users.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_menu(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	int page_id,
	boost::json::object&& context) {
	lgdebug << "view users: " << page_id << std::endl;
	
	bserv::db_transaction tx{ conn };
	bserv::db_result db_res = tx.exec("select count(*) from dish;");
	lginfo << db_res.query();
	std::size_t total_users = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total users: " << total_users << std::endl;
	int total_pages = (int)total_users / 10;
	if (total_users % 10 != 0) ++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;	
	
	db_res = tx.exec("select * from dish limit 10 offset ?;", (page_id - 1) * 10);
	lginfo << db_res.query();
	auto menus = orm_list.convert_to_vector(db_res);
	boost::json::array json_lists;
	for (auto& menu : menus) {
		json_lists.push_back(menu);
	}
	db_res = tx.exec("select distinct spiciness from dish");
	auto spicinesss = orm_spiciness.convert_to_vector(db_res);
	boost::json::array json_spicinesss;
	for (auto& spiciness : spicinesss) {
		json_spicinesss.push_back(spiciness);
	}
	db_res = tx.exec("select distinct cname from dish");
	auto cuisines = orm_cuisine.convert_to_vector(db_res);
	boost::json::array json_cuisines;
	for (auto& cuisine : cuisines) {
		json_cuisines.push_back(cuisine);
	}
	boost::json::object pagination;
	if (total_pages != 0) {
		pagination["total"] = total_pages;
		if (page_id > 1) {
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages) {
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2) {
			pagination["left_ellipsis"] = true;
		}
		else {
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1) {
			pagination["right_ellipsis"] = true;
		}
		else {
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i) {
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i) {
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["menus"] = json_lists;
	context["spicinesss"] = json_spicinesss;
	context["cuisines"] = json_cuisines;
	return index("menu.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_ordered(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	int page_id,
	boost::json::object&& context,
	boost::json::object&& params) {
	lgdebug << "view users: " << page_id << std::endl;

	bserv::db_transaction tx{ conn };
	bserv::db_result db_res = tx.exec("select count(*) from ordered;");
	lginfo << db_res.query();
	std::size_t total_users = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total users: " << total_users << std::endl;
	int total_pages = (int)total_users / 10;
	if (total_users % 10 != 0) ++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;
	bserv::session_type& session = *session_ptr;
	db_res = tx.exec("select dname,cname,uname,ocount,is_favorite from ordered,dish where dname=dishname and uname=? order by ocount desc ;", params["setuser"].as_string());
	lginfo << db_res.query();
	auto ordereds = orm_ordered.convert_to_vector(db_res);
	boost::json::array json_ordereds;
	for (auto& ordered : ordereds) {
		json_ordereds.push_back(ordered);
	}
	db_res = tx.exec("select sum(price*ocount) from ordered where uname=?;", params["setuser"].as_string());
	lginfo << db_res.query();
	auto prices = orm_price.convert_to_vector(db_res);
	boost::json::array json_prices;
	for (auto& price : prices) {
		json_prices.push_back(price);
	}

	boost::json::object pagination;
	if (total_pages != 0) {
		pagination["total"] = total_pages;
		if (page_id > 1) {
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages) {
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2) {
			pagination["left_ellipsis"] = true;
		}
		else {
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1) {
			pagination["right_ellipsis"] = true;
		}
		else {
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i) {
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i) {
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["prices"] = json_prices;
	context["ordereds"] = json_ordereds;
	return index("ordered.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_todolist(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	int page_id,
	boost::json::object&& context,
	boost::json::object&& params) {
	lgdebug << "view users: " << page_id << std::endl;

	bserv::db_transaction tx{ conn };
	bserv::db_result db_res = tx.exec("select count(*) from ordered;");
	lginfo << db_res.query();
	std::size_t total_users = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total users: " << total_users << std::endl;
	int total_pages = (int)total_users / 10;
	if (total_users % 10 != 0) ++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;
	bserv::session_type& session = *session_ptr;
	db_res = tx.exec("SELECT dname, cname, uname, ocount, is_favorite FROM ordered JOIN dish ON dname = dishname WHERE uname IN (SELECT username FROM auth_user WHERE is_active=true) ORDER BY ocount DESC;");
	lginfo << db_res.query();
	auto ordereds = orm_ordered.convert_to_vector(db_res);
	boost::json::array json_ordereds;
	for (auto& ordered : ordereds) {
		json_ordereds.push_back(ordered);
	}
	
	boost::json::object pagination;
	if (total_pages != 0) {
		pagination["total"] = total_pages;
		if (page_id > 1) {
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages) {
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2) {
			pagination["left_ellipsis"] = true;
		}
		else {
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1) {
			pagination["right_ellipsis"] = true;
		}
		else {
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i) {
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i) {
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["ordereds"] = json_ordereds;
	return index("todolist.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_more(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	int page_id,
	boost::json::object&& context,
	boost::json::object&& params) {
	lgdebug << "view users: " << page_id << std::endl;

	bserv::db_transaction tx{ conn };
	bserv::db_result db_res = tx.exec("select count(*) from ordered;");
	lginfo << db_res.query();
	std::size_t total_users = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total users: " << total_users << std::endl;
	int total_pages = (int)total_users / 10;
	if (total_users % 10 != 0) ++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;
	bserv::session_type& session = *session_ptr;
	db_res = tx.exec("select cname,area,message,sample from cuisines where cname=?;", params["colcuisine"].as_string());
	lginfo << db_res.query();
	auto mores = orm_more.convert_to_vector(db_res);
	boost::json::array json_mores;
	for (auto& ordered : mores) {
		json_mores.push_back(ordered);
	}
	db_res = tx.exec("select dishname from dish where cname=?;", params["colcuisine"].as_string());
	auto dishs = orm_dish.convert_to_vector(db_res);
	boost::json::array json_dishs;
	for (auto& ordered : dishs) {
		json_dishs.push_back(ordered);
	}
	boost::json::object pagination;
	if (total_pages != 0) {
		pagination["total"] = total_pages;
		if (page_id > 1) {
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages) {
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2) {
			pagination["left_ellipsis"] = true;
		}
		else {
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1) {
			pagination["right_ellipsis"] = true;
		}
		else {
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i) {
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i) {
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["mores"] = json_mores;
	context["dishs"] = json_dishs;
	return index("more.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_rec(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	int page_id,
	boost::json::object&& context,
	boost::json::object&& params) {
	lgdebug << "view users: " << page_id << std::endl;

	bserv::db_transaction tx{ conn };
	bserv::db_result db_res = tx.exec("select count(*) from ordered;");
	lginfo << db_res.query();
	std::size_t total_users = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total users: " << total_users << std::endl;
	int total_pages = (int)total_users / 10;
	if (total_users % 10 != 0) ++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;
	bserv::session_type& session = *session_ptr;
	db_res = tx.exec("select x.id,x.dishname,x.price,x.spiciness,x.cname from dish as x where x.cname in (select y.cname from dish as y,ordered as z where z.uname=? and y.dishname=z.dname group by y.cname having sum(z.ocount)>=all (select sum (ocount) from dish,ordered where dish.dishname=ordered.dname and ordered.uname=? group by dish.cname) ) and x.dishname not in(select dname from ordered where uname=?);"
		, get_or_empty(params,"setuser"), get_or_empty(params,"setuser"), get_or_empty(params,"setuser"));
	lginfo << db_res.query();
	auto menus = orm_list.convert_to_vector(db_res);
	boost::json::array json_lists;
	for (auto& ordered : menus) {
		json_lists.push_back(ordered);
	}
	db_res = tx.exec("select y.cname from dish as y,ordered as z where z.uname=? and y.dishname=z.dname group by y.cname having sum(z.ocount)>=all (select sum (ocount) from dish,ordered where dish.dishname=ordered.dname and ordered.uname=? group by dish.cname) limit 10 offset ?;"
		, params["setuser"].as_string(), params["setuser"].as_string(), (page_id - 1) * 10);
	auto lists1 = orm_cuisine.convert_to_vector(db_res);
	boost::json::array json_lists1;
	for (auto& ordered : lists1) {
		json_lists1.push_back(ordered);
	}
	
	db_res = tx.exec("select x.id,x.dishname,x.price,x.spiciness,x.cname from dish as x where x.spiciness in (select y.spiciness from dish as y,ordered as z where z.uname=? and y.dishname=z.dname group by y.spiciness having sum(z.ocount)>=all (select sum (ocount) from dish,ordered where dish.dishname=ordered.dname and ordered.uname=? group by dish.spiciness) ) and x.dishname not in(select dname from ordered where uname=?) limit 10 offset ?;"
		, params["setuser"].as_string(), params["setuser"].as_string(), params["setuser"].as_string(), (page_id - 1) * 10);
	auto lists2 = orm_list.convert_to_vector(db_res);
	boost::json::array json_lists2;
	for (auto& ordered : lists2) {
		json_lists2.push_back(ordered);
	}

	db_res = tx.exec("select y.spiciness from dish as y,ordered as z where z.uname=? and y.dishname=z.dname group by y.spiciness having sum(z.ocount)>=all (select sum (ocount) from dish,ordered where dish.dishname=ordered.dname and ordered.uname=? group by dish.spiciness)limit 10 offset ?;"
		, params["setuser"].as_string(), params["setuser"].as_string(), (page_id - 1) * 10);
	auto lists3 = orm_spiciness.convert_to_vector(db_res);
	boost::json::array json_lists3;
	for (auto& ordered : lists3) {
		json_lists3.push_back(ordered);
	}
	db_res = tx.exec("select x.id,x.dishname,x.price,x.spiciness,x.cname from dish as x ,ordered as y where x.dishname=y.dname and y.is_favorite=true and y.uname=? limit 10 offset ?;"
		, params["setuser"].as_string(),  (page_id - 1) * 10);
	auto lists4 = orm_list.convert_to_vector(db_res);
	boost::json::array json_lists4;
	for (auto& ordered : lists4) {
		json_lists4.push_back(ordered);
	}
	boost::json::object pagination;
	if (total_pages != 0) {
		pagination["total"] = total_pages;
		if (page_id > 1) {
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages) {
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2) {
			pagination["left_ellipsis"] = true;
		}
		else {
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1) {
			pagination["right_ellipsis"] = true;
		}
		else {
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i) {
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i) {
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["rec1s"] = json_lists;
	context["rec1_cuisine"] = json_lists1;
	context["rec2s"] = json_lists2;
	context["rec2_spiciness"] = json_lists3;
	context["rec3s"] = json_lists4;
	return index("recommandation.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_spiciness(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	int page_id,
	boost::json::object&& context,
	boost::json::object&& params) {
	lgdebug << "view users: " << page_id << std::endl;
	bserv::db_transaction tx{ conn };
	bserv::db_result db_res = tx.exec("select count(*) from dish where spiciness=?;",params["spicinessback"].as_string());
	lginfo << db_res.query();
	std::size_t total_users = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total users: " << total_users << std::endl;
	int total_pages = (int)total_users / 10;
	if (total_users % 10 != 0) ++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;
	db_res = tx.exec("select * from dish where spiciness=? limit 10 offset ? ;",params["spicinessback"].as_string(), (page_id - 1) * 10);
	lginfo << db_res.query();
	auto menus = orm_list.convert_to_vector(db_res);
	boost::json::array json_lists;
	for (auto& menu : menus) {
		json_lists.push_back(menu);
	}
	db_res = tx.exec("select distinct spiciness from dish");
	auto spicinesss = orm_spiciness.convert_to_vector(db_res);
	boost::json::array json_spicinesss;
	for (auto& spiciness : spicinesss) {
		json_spicinesss.push_back(spiciness);
	}
	db_res = tx.exec("select distinct cname from dish");
	auto cuisines = orm_cuisine.convert_to_vector(db_res);
	boost::json::array json_cuisines;
	for (auto& cuisine : cuisines) {
		json_cuisines.push_back(cuisine);
	}
	boost::json::object pagination;
	if (total_pages != 0) {
		pagination["total"] = total_pages;
		if (page_id > 1) {
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages) {
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2) {
			pagination["left_ellipsis"] = true;
		}
		else {
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1) {
			pagination["right_ellipsis"] = true;
		}
		else {
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i) {
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i) {
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["menus"] = json_lists;
	context["spicinesss"] = json_spicinesss;
	context["cuisines"] = json_cuisines;
	return index("menu.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_search(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	int page_id,
	boost::json::object&& context,
	boost::json::object&& params) {
	lgdebug << "view users: " << page_id << std::endl;
	bserv::db_transaction tx{ conn };
	bserv::db_result db_res = tx.exec("select count(*) from dish where dishname LIKE ?;", params["search"].as_string());
	lginfo << db_res.query();
	std::size_t total_users = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total users: " << total_users << std::endl;
	int total_pages = (int)total_users / 10;
	if (total_users % 10 != 0) ++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;
	db_res = tx.exec("select * from dish where dishname LIKE ? limit 10 offset ? ;", params["search"].as_string(), (page_id - 1) * 10);
	lginfo << db_res.query();
	auto menus = orm_list.convert_to_vector(db_res);
	boost::json::array json_lists;
	for (auto& menu : menus) {
		json_lists.push_back(menu);
	}
	db_res = tx.exec("select distinct spiciness from dish");
	auto spicinesss = orm_spiciness.convert_to_vector(db_res);
	boost::json::array json_spicinesss;
	for (auto& spiciness : spicinesss) {
		json_spicinesss.push_back(spiciness);
	}
	db_res = tx.exec("select distinct cname from dish");
	auto cuisines = orm_cuisine.convert_to_vector(db_res);
	boost::json::array json_cuisines;
	for (auto& cuisine : cuisines) {
		json_cuisines.push_back(cuisine);
	}
	boost::json::object pagination;
	if (total_pages != 0) {
		pagination["total"] = total_pages;
		if (page_id > 1) {
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages) {
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2) {
			pagination["left_ellipsis"] = true;
		}
		else {
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1) {
			pagination["right_ellipsis"] = true;
		}
		else {
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i) {
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i) {
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["menus"] = json_lists;
	context["spicinesss"] = json_spicinesss;
	context["cuisines"] = json_cuisines;
	return index("menu.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_cuisine(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	int page_id,
	boost::json::object&& context,
	boost::json::object&& params) {
	lgdebug << "view users: " << page_id << std::endl;
	bserv::db_transaction tx{ conn };
	bserv::db_result db_res = tx.exec("select count(*) from dish where cname=? ;", params["cuisineback"].as_string());
	lginfo << db_res.query();
	std::size_t total_users = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total users: " << total_users << std::endl;
	int total_pages = (int)total_users / 10;
	if (total_users % 10 != 0) ++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;
	db_res = tx.exec("select * from dish where cname=? limit 10 offset ? ;", params["cuisineback"].as_string(), (page_id - 1) * 10);
	lginfo << db_res.query();
	auto menus = orm_list.convert_to_vector(db_res);
	boost::json::array json_lists;
	for (auto& menu : menus) {
		json_lists.push_back(menu);
	}
	db_res = tx.exec("select distinct cname from dish");
	auto cuisines = orm_cuisine.convert_to_vector(db_res);
	boost::json::array json_cuisines;
	for (auto& cuisine : cuisines) {
		json_cuisines.push_back(cuisine);
	}
	lgdebug << json_cuisines.size();
	db_res = tx.exec("select distinct spiciness from dish");
	auto spicinesss = orm_spiciness.convert_to_vector(db_res);
	boost::json::array json_spicinesss;
	for (auto& spiciness : spicinesss) {
		json_spicinesss.push_back(spiciness);
	}
	boost::json::object pagination;
	if (total_pages != 0) {
		pagination["total"] = total_pages;
		if (page_id > 1) {
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages) {
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2) {
			pagination["left_ellipsis"] = true;
		}
		else {
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1) {
			pagination["right_ellipsis"] = true;
		}
		else {
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i) {
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i) {
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["menus"] = json_lists;
	context["cuisines"] = json_cuisines;
	context["spicinesss"] = json_spicinesss;
	return index("menu.html", session_ptr, response, context);
}

std::nullopt_t view_users(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	const std::string& page_num) {
	int page_id = std::stoi(page_num);
	boost::json::object context;
	return redirect_to_users(conn, session_ptr, response, page_id, std::move(context));
}

std::nullopt_t view_list(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	const std::string& page_num) {
	int page_id = std::stoi(page_num);
	boost::json::object context;
	return redirect_to_menu(conn, session_ptr, response, page_id, std::move(context));
}

std::nullopt_t view_ordered(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	boost::json::object&& params,
	const std::string& page_num) {
	int page_id = std::stoi(page_num);
	boost::json::object context;
	return redirect_to_ordered(conn, session_ptr, response, page_id, std::move(context), std::move(params));
}

std::nullopt_t view_todolist(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	boost::json::object&& params,
	const std::string& page_num) {
	int page_id = std::stoi(page_num);
	boost::json::object context;
	return redirect_to_todolist(conn, session_ptr, response, page_id, std::move(context), std::move(params));
}

std::nullopt_t view_rec(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	boost::json::object&& params,
	const std::string& page_num) {
	int page_id = std::stoi(page_num);
	boost::json::object context;
	return redirect_to_rec(conn, session_ptr, response, page_id, std::move(context), std::move(params));
}

std::nullopt_t view_spiciness(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	boost::json::object&& params,
	const std::string& page_num) {
	int page_id = std::stoi(page_num);
	boost::json::object context;
	return redirect_to_spiciness(conn, session_ptr, response, page_id, std::move(context),std::move(params));
}

std::nullopt_t view_search(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	boost::json::object&& params,
	const std::string& page_num) {
	int page_id = std::stoi(page_num);
	boost::json::object context;
	return redirect_to_search(conn, session_ptr, response, page_id, std::move(context), std::move(params));
}



std::nullopt_t view_cuisine(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	boost::json::object&& params,
	const std::string& page_num) {
	int page_id = std::stoi(page_num);
	boost::json::object context;
	return redirect_to_cuisine(conn, session_ptr, response, page_id, std::move(context), std::move(params));
}

std::nullopt_t form_add_user(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = user_register(request, std::move(params), conn);
	return redirect_to_users(conn, session_ptr, response, 1, std::move(context));
}
std::nullopt_t form_add_menu(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = menu_register(request, std::move(params), conn);
	return redirect_to_menu(conn, session_ptr, response, 1, std::move(context));
}
std::nullopt_t form_add_cuisine(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = cuisine_register(request, std::move(params), conn);
	return redirect_to_menu(conn, session_ptr, response, 1, std::move(context));
}
std::nullopt_t delete_user(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = user_delete(request, std::move(params), conn);
	return redirect_to_users(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t delete_menu(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = menu_delete(request, std::move(params), conn);
	return redirect_to_menu(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t orderin(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = menu_orderin(request, std::move(params), conn);
	return redirect_to_menu(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t view_more(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context ;
	return redirect_to_more(conn, session_ptr, response, 1, std::move(context), std::move(params));
}

std::nullopt_t set_favor(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = favor_set(request, std::move(params), conn);
	return redirect_to_ordered(conn, session_ptr, response, 1, std::move(context), std::move(params));
}

std::nullopt_t play(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = play_set(request, std::move(params), conn);
	return redirect_to_ordered(conn, session_ptr, response, 1, std::move(context), std::move(params));
}

std::nullopt_t clear(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = clear_set(request, std::move(params), conn);
	return redirect_to_ordered(conn, session_ptr, response, 1, std::move(context), std::move(params));
}

std::nullopt_t dlt(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = delete_set(request, std::move(params), conn);
	return redirect_to_ordered(conn, session_ptr, response, 1, std::move(context), std::move(params));
}

std::nullopt_t todo_set_favor(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = td_favor_set(request, std::move(params), conn);
	return redirect_to_todolist(conn, session_ptr, response, 1, std::move(context), std::move(params));
}

std::nullopt_t todo_play(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = td_play_set(request, std::move(params), conn);
	return redirect_to_todolist(conn, session_ptr, response, 1, std::move(context), std::move(params));
}

std::nullopt_t todo_clear(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = td_clear_set(request, std::move(params), conn);
	return redirect_to_todolist(conn, session_ptr, response, 1, std::move(context), std::move(params));
}

std::nullopt_t todo_dlt(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = td_delete_set(request, std::move(params), conn);
	return redirect_to_todolist(conn, session_ptr, response, 1, std::move(context), std::move(params));
}