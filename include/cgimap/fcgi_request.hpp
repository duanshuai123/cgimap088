#ifndef FCGI_REQUEST_HPP
#define FCGI_REQUEST_HPP

#include "cgimap/request.hpp"
#include <chrono>
#include <memory>

struct fcgi_request : public request {
  fcgi_request(int socket, const std::chrono::system_clock::time_point &now);
  virtual ~fcgi_request();
  const char *get_param(const char *key) const override;
  const std::string get_payload() override;

  // getting and setting the current time
  std::chrono::system_clock::time_point get_current_time() const override;
  // need to be able to set the time, since the fcgi_request is
  // actually wrapping the whole socket and so persists over
  // several calls.
  void set_current_time(const std::chrono::system_clock::time_point &now);

  int accept_r();
  static int open_socket(const std::string &, int);
  void dispose() override;

protected:
  void write_header_info(int status, const request::headers_t &headers) override;
  output_buffer& get_buffer_internal() override;
  void finish_internal() override;

private:
  struct pimpl;
  std::unique_ptr<pimpl> m_impl;
  std::unique_ptr<output_buffer> m_buffer;
};

#endif /* FCGI_REQUEST_HPP */
