#include "crow_all.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <boost/filesystem.hpp>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/oid.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;
using mongocxx::cursor;

using namespace crow;
using namespace std;
using namespace crow::mustache;

string getView(const string &filename, context &x)
{
	cout << filename << endl;
	cout << "/usr/src/cppweb/cppbox/hello_crow/public/" + filename + ".html" << endl;
	return load("/usr/src/cppweb/cppbox/hello_crow/public/" + filename + ".html").render(x);
}

void sendFile(response &res, string fileName, string contentType)
{
	ifstream in("/usr/src/cppweb/cppbox/hello_crow/public/" + fileName, ifstream::in);
	if (in)
	{
		ostringstream contents;
		contents << in.rdbuf();
		in.close();
		res.set_header("Content-Type", contentType);
		res.write(contents.str());
	}
	else
	{
		res.code = 404;
		res.write("File not found 404");
	}
	res.end();
}

void sendHtml(response &res, string fileName)
{
	sendFile(res, fileName + ".html", "text/html");
}

void sendImage(response &res, string fileName)
{
	sendFile(res, "images/" + fileName, "imag/jpeg");
}

void sendScripts(response &res, string fileName)
{
	sendFile(res, "scripts/" + fileName, "text/javascript");
}

void sendStyle(response &res, string fileName)
{
	sendFile(res, "styles/" + fileName, "text/css");
}

int main(int args, char *argv[])
{
	crow::SimpleApp app;
	// mustache will not work without set_base
	crow::mustache::set_base(".");

	mongocxx::instance inst{};
	string mongoConnect = std::string(getenv("MONGODB_URI"));
	mongocxx::client conn{mongocxx::uri{mongoConnect}};
	auto collection = conn["cppcontacts"]["contacts"];

	CROW_ROUTE(app, "/styles/<string>")
	([](const request &req, response &res, string fileName)
	 { sendStyle(res, fileName); });

	CROW_ROUTE(app, "/scripts/<string>")
	([](const request &req, response &res, string fileName)
	 { sendScripts(res, fileName); });

	CROW_ROUTE(app, "/images/<string>")
	([](const request &req, response &res, string fileName)
	 { sendImage(res, fileName); });

	CROW_ROUTE(app, "/contacts")
	([&collection]()
	 {
      mongocxx::options::find opts;
      opts.skip(9);
      opts.limit(10);
      auto docs = collection.find({}, opts);
      crow::json::wvalue dto;
      vector<crow::json::rvalue> contacts;
      contacts.reserve(10);

      for(auto doc : docs){
        contacts.push_back(json::load(bsoncxx::to_json(doc)));
      }
      dto["contacts"] = contacts;
      return getView("contacts", dto); });

	CROW_ROUTE(app, "/")
	([](const request &req, response &res)
	 { sendHtml(res, "index"); });

	CROW_ROUTE(app, "/about")
	([](const request &req, response &res)
	 { sendHtml(res, "about"); });

	char *port = getenv("PORT");
	uint16_t iPort = static_cast<uint16_t>(port != NULL ? stoi(port) : 18080);
	cout << "Port" << iPort << endl;
	app.port(iPort).multithreaded().run();
}
