// Copyright (c) 2018-2021 Dr. Colin Hirsch and Daniel Frey
// Please see LICENSE for license or visit https://github.com/taocpp/config/

#ifndef TAO_CONFIG_INTERNAL_MEMBER_FUNCTIONS_HPP
#define TAO_CONFIG_INTERNAL_MEMBER_FUNCTIONS_HPP

#include <cerrno>
#include <string>
#include <system_error>

#include "concat.hpp"
#include "config_action.hpp"
#include "config_grammar.hpp"
#include "extension_maps.hpp"
#include "forward.hpp"
#include "key1.hpp"
#include "pegtl.hpp"
#include "phase1_append.hpp"
#include "state.hpp"
#include "system_utility.hpp"

namespace tao::config::internal
{
   inline void include_function( state& st, const extension_maps& em, const pegtl::position& p, const std::string& f )
   {
      try {
         pegtl::file_input in( f );
         pegtl::parse_nested< rules::config_file, config_action >( p, static_cast< pegtl_input_t& >( in ), st, em );
      }
      catch( const std::system_error& e ) {
         throw pegtl::parse_error( "include error", p );
         // throw pegtl::parse_error( format( __FILE__, __LINE__, "include failed", { { "filename", f }, { "error", e.what() }, { "errno", e.code().value() } } ), pos );
      }
   }

   inline void include_if_function( state& st, const extension_maps& em, const pegtl::position& p, const std::string& f )
   {
      try {
         pegtl::file_input in( f );
         pegtl::parse_nested< rules::config_file, config_action >( p, static_cast< pegtl_input_t& >( in ), st, em );
      }
      catch( const std::system_error& e ) {
         if( e.code().value() != ENOENT ) {
            throw pegtl::parse_error( "include error -- file not found", p );
            // throw pegtl::parse_error( format( __FILE__, __LINE__, "include failed", { { "filename", f }, { "error", e.what() }, { "errno", e.code().value() } } ), pos );
         }
      }
   }

   inline void member_function( state& st, const extension_maps& em, const pegtl::position& p, const std::string& c )
   {
      pegtl::memory_input in( c, __FUNCTION__ );
      pegtl::parse_nested< rules::config_file, config_action >( p, static_cast< pegtl_input_t& >( in ), st, em );
   }

   inline void schema_function( state& st, const std::optional< std::string >& s )
   {
      assert( st.suffix.empty() );

      if( st.prefix.empty() ) {
         st.schema = s ? ( *s ) : std::string();
      }
      else {
         const auto f = [ & ]( concat& c ) { c.schema = s ? ( *s ) : std::string(); };
         phase1_append( st.root, st.prefix, f, !s.has_value() );
      }
   }

   inline void setenv_function( const pegtl::position& p, const std::string& name, const std::string& value )
   {
#if defined( _MSC_VER )
      throw pegtl::parse_error( "setenv not supported on this platform", p );
#else
      set_env_throws( p, name, value );
#endif
   }

}  // namespace tao::config::internal

#endif
