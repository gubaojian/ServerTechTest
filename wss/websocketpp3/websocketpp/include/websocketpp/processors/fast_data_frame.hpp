//
//  client_data_frame_prepare.hpp
//  websocketpp
//
//  Created by baojian on 2025/6/11.
//
#ifndef WSG_BUS_DATA_FRAME_HPP
#define WSG_BUS_DATA_FRAME_HPP

#include <websocketpp/config/core.hpp>

#include <websocketpp/processors/base.hpp>
#include <websocketpp/common/system_error.hpp>

#include <websocketpp/close.hpp>
#include <websocketpp/utilities.hpp>
#include <websocketpp/uri.hpp>

#include <websocketpp/processors/processor.hpp>


#include <websocketpp/frame.hpp>
#include <websocketpp/http/constants.hpp>

#include <websocketpp/utf8_validator.hpp>
#include <websocketpp/sha1/sha1.hpp>
#include <websocketpp/base64/base64.hpp>

#include <websocketpp/common/network.hpp>
#include <websocketpp/common/platforms.hpp>

#include <websocketpp/message_buffer/alloc.hpp>
#include <websocketpp/message_buffer/message.hpp>

#include <algorithm>
#include <cassert>
#include <string>
#include <vector>
#include <utility>
#include <thread>

namespace websocketpp {
    namespace processor {
    
    typedef websocketpp::message_buffer::message<message_buffer::alloc::con_msg_manager>
        message_type;
    typedef typename message_type::ptr message_ptr;
    
    typedef websocketpp::concurrency::basic concurrency_type;
    
    // RNG policies
    typedef websocketpp::random::random_device::int_generator_none_lock<uint32_t,
        concurrency_type> rng_type;

    
    thread_local char _buffer[64*1024]; //websocket最大消息64kb
    thread_local rng_type _m_rng;
    
    /**
     * will move paypoad frame in to avoid copy. you  should not use in payload after call this method.
     */
    lib::error_code prepare_data_frame_fast(message_ptr& in, message_ptr& out) {
        if (!in) {
            return make_error_code(error::invalid_arguments);
        }

        frame::opcode::value op = in->get_opcode();

        // validate opcode: only regular data frames
        if (frame::opcode::is_control(op)) {
            return make_error_code(error::invalid_opcode);
        }

        std::string& i = in->get_raw_payload();
        
       // validate payload utf8
        if (op == frame::opcode::TEXT && !utf8_validator::validate(i)) {
            return make_error_code(error::invalid_payload);
        }
        
        if (in->get_compressed()) {
            return make_error_code(error::not_implemented);
        }

        frame::masking_key_type key;
        
        bool fin = in->get_fin();
        
        key.i = _m_rng();
      
        size_t osize =  i.size();
        
        frame::basic_header h(op, osize, fin, true, false);
        frame::extended_header e(osize, key.i);
        
        size_t hsize = frame::get_header_len(h);
        const char* header = _buffer;
        frame::prepare_header(h, e);
        
        frame::masked_copy_simd64(i, i, key);
        
        out = lib::make_shared<message_type>(std::move(i), header, hsize);
 

        out->set_prepared(true);
        out->set_opcode(op);

        return lib::error_code();
    }
    }
    
   
}

#endif
