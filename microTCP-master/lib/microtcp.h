/*
 * microtcp, a lightweight implementation of TCP for teaching,
 * and academic purposes.
 *
 * Copyright (C) 2015-2017  Manolis Surligas <surligas@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIB_MICROTCP_H_
#define LIB_MICROTCP_H_
#include <arpa/inet.h>  //inet_addr
#include <netdb.h>      //sockaddr
#include <stdint.h>
#include <stdio.h>  //new include for printf
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h> 
 
/*
 * Several useful constants
 */
#define MICROTCP_ACK_TIMEOUT_US 200000
#define MICROTCP_MSS 1400
#define MICROTCP_RECVBUF_LEN 8192
#define MICROTCP_WIN_SIZE MICROTCP_RECVBUF_LEN
#define MICROTCP_INIT_CWND (3 * MICROTCP_MSS)
#define MICROTCP_INIT_SSTHRESH MICROTCP_WIN_SIZE

/*control bits*/
#define ACK 4096       // 0001000000000000
#define RST 8192       // 0010000000000000
#define SYN 16384      // 0100000000000000
#define FIN 32768      // 1000000000000000
#define SYN_ACK 20480  // 0101000000000000
#define FIN_ACK 36864  // 1001000000000000

/**
 * Possible states of the microTCP socket
 *
 * NOTE: You can insert any other possible state
 * for your own convenience
 */
typedef enum {
  UNKNOWN,
  LISTEN,
  ESTABLISHED,
  CLOSING_BY_PEER,
  CLOSING_BY_HOST,
  CLOSED,
  INVALID
} mircotcp_state_t;

/**
 * This is the microTCP socket structure. It holds all the necessary
 * information of each microTCP socket.
 *
 * NOTE: Fill free to insert additional fields.
 */
typedef struct {
  int sd;                 /**< The underline UDP socket descriptor */
  mircotcp_state_t state; /**< The state of the microTCP socket */
  size_t
      init_win_size; /**< The window size negotiated at the 3-way handshake */
  size_t curr_win_size; /**< The current window size */

  uint8_t *recvbuf;      /**< The *receive* buffer of the TCP
                              connection. It is allocated during the connection
                            establishment and          is freed at the shutdown of
                            the connection.
                            This buffer is used          to retrieve the data from
                            the network. */
  size_t buf_fill_level; /**< Amount of data in the buffer */

  size_t cwnd;
  size_t ssthresh;

  size_t seq_number; /**< Keep the state of the sequence number */
  size_t ack_number; /**< Keep the state of the ack number */
  size_t left_sack;  /**< Keep the state of the left sack number */
  size_t right_sack; /**< Keep the state of the right sack number */
  uint64_t packets_send;
  uint64_t packets_received;
  uint64_t packets_lost;
  uint64_t bytes_send;
  uint64_t bytes_received;
  uint64_t bytes_lost;

  /*Additional fields*/
  struct sockaddr_in *sin;
  socklen_t address_len;
  int index;
  int poll_flag;
  double rx_min_inter;
  double rx_max_inter;
  double rx_mean_inter;
  double tx_min_inter;
  double tx_max_inter;
  double tx_mean_inter;
  struct timespec last_sent;
  struct timespec last_rcvd;

} microtcp_sock_t;

/**
 * microTCP header structure
 * NOTE: DO NOT CHANGE!
 */
typedef struct {
  uint32_t seq_number;  /**< Sequence number */
  uint32_t ack_number;  /**< ACK number */
  uint16_t control;     /**< Control bits (e.g. SYN, ACK, FIN) */
  uint16_t window;      /**< Window size in bytes */
  uint32_t data_len;    /**< Data length in bytes (EXCLUDING header) */
  uint32_t future_use0; /**< 32-bits for future use */
  uint32_t future_use1; /**< 32-bits for future use */
  uint32_t future_use2; /**< 32-bits for future use */
  uint32_t checksum;    /**< CRC-32 checksum, see crc32() in utils folder */
  uint32_t left_sack;   /**< Left edge SACK number */
  uint32_t right_sack;  /**< Right edge SACK number */
} microtcp_header_t;

microtcp_sock_t microtcp_socket(int domain, int type, int protocol);

int microtcp_bind(microtcp_sock_t *socket, const struct sockaddr *address,
                  socklen_t address_len);

int microtcp_connect(microtcp_sock_t *socket, const struct sockaddr *address,
                     socklen_t address_len);

/**
 * Blocks waiting for a new connection from a remote peer.
 *
 * @param socket the socket structure
 * @param address pointer to store the address information of the connected peer
 * @param address_len the length of the address structure.
 * @return ATTENTION despite the original accept() this function returns
 * 0 on success or -1 on failure
 */
int microtcp_accept(microtcp_sock_t *socket, struct sockaddr *address,
                    socklen_t address_len);

int microtcp_shutdown(microtcp_sock_t *socket, int how);

ssize_t microtcp_send(microtcp_sock_t *socket, const void *buffer,
                      size_t length, int flags);

ssize_t microtcp_recv(microtcp_sock_t *socket, void *buffer, size_t length,
                      int flags);

#endif /* LIB_MICROTCP_H_ */
