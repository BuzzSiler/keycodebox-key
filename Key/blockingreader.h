#ifndef CBLOCKINGREADER_H
#define CBLOCKINGREADER_H



#include <boost/asio/serial_port.hpp>
#include <boost/bind.hpp>


/**
 * @brief The CBlockingReader class
 * @example_usage:
 *
 * #include <string>
 * #include <boost/asio/serial_port.hpp>
 * #include <boost/asio.hpp>
 * #include "blocking_reader.h"
 *
 * using namespace boost;
 *
 * std::string read_response() {
 *
 *     ... // open port code
 *
 * 	// A blocking reader for this port that
 * 	// will time out a read after 500 milliseconds.
 * 	CBlockingReader reader(port, 500);
 *
 * 	char c;
 * 	std::string rsp;
 *
 * 	// read from the serial port until we get a
 * 	// \n or until a read times-out (500ms)
 * 	while (reader.read_char(c) && c != '\n') {
 * 		rsp += c;
 * 	}
 *
 * 	if (c != '\n') {
 * 		// it must have timed out.
 * 		throw std::exception("Read timed out!");
 * 	}
 *
 * 	return rsp;
 * }
 */
class CBlockingReader
{
    boost::asio::serial_port& port;
    size_t timeout;
    char c;
    boost::asio::deadline_timer timer;
    bool read_error;

    // Called when an async read completes or has been cancelled
    void readComplete(const boost::system::error_code& error,
                        size_t bytes_transferred) {

        read_error = (error || bytes_transferred == 0);

        // Read has finished, so cancel the
        // timer.
        timer.cancel();
    }

    // Called when the timer's deadline expires.
    void timeOut(const boost::system::error_code& error) {

        // Was the timeout was cancelled?
        if (error) {
            // yes
            return;
        }

        // no, we have timed out, so kill
        // the read operation
        // The read callback will be called
        // with an error
        port.cancel();
    }

public:

    // Constructs a blocking reader, pass in an open serial_port and
    // a timeout in milliseconds.
    CBlockingReader(boost::asio::serial_port& port, size_t timeout) :
                                                port(port), timeout(timeout),
                                                timer(port.get_io_service()),
                                                read_error(true) {

    }

    // Reads a character or times out
    // returns false if the read times out
    bool readChar(char& val) {

        val = c = '\0';

        // After a timeout & cancel it seems we need
        // to do a reset for subsequent reads to work.
        port.get_io_service().reset();

        // Asynchronously read 1 character.
        boost::asio::async_read(port, boost::asio::buffer(&c, 1),
                boost::bind(&CBlockingReader::readComplete,
                        this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));

        // Setup a deadline time to implement our timeout.
        timer.expires_from_now(boost::posix_time::milliseconds(timeout));
        timer.async_wait(boost::bind(&CBlockingReader::timeOut,
                                this, boost::asio::placeholders::error));

        // This will block until a character is read
        // or until the it is cancelled.
        port.get_io_service().run();

        if (!read_error)
            val = c;

        return !read_error;
    }
};

#endif //  CBLOCKINGREADER_H
