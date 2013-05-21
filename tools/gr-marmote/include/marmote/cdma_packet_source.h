/* -*- c++ -*- */
/* 
 * Copyright 2013 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */


#ifndef INCLUDED_MARMOTE_CDMA_PACKET_SOURCE_H
#define INCLUDED_MARMOTE_CDMA_PACKET_SOURCE_H

#include <marmote/api.h>
#include <gr_block.h>

namespace gr {
  namespace marmote {

    /*!
     * \brief <+description of block+>
     * \ingroup marmote
     *
     */
    class MARMOTE_API cdma_packet_source : virtual public gr_block
    {
     public:
      typedef boost::shared_ptr<cdma_packet_source> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of marmote::cdma_packet_source.
       *
       * To avoid accidental use of raw pointers, marmote::cdma_packet_source's
       * constructor is in a private implementation
       * class. marmote::cdma_packet_source::make is the public interface for
       * creating new instances.
       */
      static sptr make(bool debug, unsigned int preamble_length, unsigned int payload_length);
    };

  } // namespace marmote
} // namespace gr

#endif /* INCLUDED_MARMOTE_CDMA_PACKET_SOURCE_H */

