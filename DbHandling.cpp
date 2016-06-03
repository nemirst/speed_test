/* Standard C++ includes */
#include <stdlib.h>
#include <iostream>
#include <sstream>

#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include "DbHandling.h"

DbHandling::DbHandling(const std::map<std::string, std::string>& c) {
	this->conf = c;
}

DbHandling::~DbHandling() {
    //dtor
}

int DbHandling::Store(const std::map<std::string, std::string>& _params) {
	using namespace std;
	using namespace sql::mysql;
	std::map<std::string, std::string> params = _params;

	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;

		driver = get_driver_instance();
		con = driver->connect("tcp://" + conf["DB_IP"] + ":" + conf["DB_PORT"], conf["DB_USER"], conf["DB_PSW"]);
		con->setSchema(conf["DB_NAME"]);

		stmt = con->createStatement();
		std::stringstream queryStream;
		queryStream << "INSERT INTO " << conf["DB_TABLE"] << "(externalIP, uploadspeed, uploadspeedtime, "
			"uploadspeeddate, downloadspeed, downloadspeedtime, downloadspeeddate, latency) VALUES('" <<
			params["EXT_IP"] << "', '" << params["UPLOAD_SPEED"] << "', '" << params["UPLOAD_DURATION"] <<
			"', '" << params["UPLOAD_TIME"] << "', '" << params["DOWNLOAD_SPEED"] << "', '" << params["DOWNLOAD_DURATION"] <<
			"', '" << params["DOWNLOAD_TIME"] << "', '" << params["LATENCY"] << "')";
		stmt->execute(queryStream.str());
		delete stmt;
		delete con;

	}
	catch (sql::SQLException &e) {
		cout << endl << "ERROR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "
			<< __LINE__ << endl;
		cout << "ERROR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
		return -1;
	}

	return 0;
}