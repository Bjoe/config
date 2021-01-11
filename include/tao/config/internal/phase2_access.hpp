// Copyright (c) 2018-2020 Dr. Colin Hirsch and Daniel Frey
// Please see LICENSE for license or visit https://github.com/taocpp/config/

#ifndef TAO_CONFIG_INTERNAL_PHASE2_ACCESS_HPP
#define TAO_CONFIG_INTERNAL_PHASE2_ACCESS_HPP

#include <cassert>
#include <iterator>
#include <string>

#include "array.hpp"
#include "concat.hpp"
#include "constants.hpp"
#include "entry.hpp"
#include "key1.hpp"
#include "object.hpp"

namespace tao::config::internal
{
   struct phase2_access_return
   {};

   [[nodiscard]] inline const concat* phase2_access( const concat& c, const key1& suffix, const int down );

   [[nodiscard]] inline const concat* phase2_access_name( const concat& c, const std::string& name, const key1& suffix, const int down )
   {
      if( c.concat.empty() ) {
         if( down >= 0 ) {
            return nullptr;
         }
         throw phase2_access_return();  // TODO: Or error or return?
      }
      if( c.concat.size() > 1 ) {
         throw phase2_access_return();
      }
      const auto& e = c.concat.front();
      switch( e.kind() ) {
         case entry_kind::value:
            throw std::string( "access name in value" );
         case entry_kind::reference:
            throw phase2_access_return();
         case entry_kind::array:
            throw std::string( "access name in array" );
         case entry_kind::object:
            if( const auto i = e.get_object().object.find( name ); i != e.get_object().object.end() ) {
               return phase2_access( i->second, suffix, down - 1 );
            }
            if( down >=0 ) {
               return nullptr;
            }
            throw std::string( "name not found" );  // NOTE: Just like all other throw std::string( ... ) this is just a temporary placeholder during refactoring.
      }
      assert( false );  // UNREACHABLE
   }

   [[nodiscard]] inline const concat* phase2_access_index( const concat& c, const std::size_t index, const key1& suffix, const int down )
   {
      if( c.concat.empty() ) {
         if( down >= 0 ) {
            return nullptr;
         }
         throw phase2_access_return();  // TODO: Or error or return?
      }
      if( c.concat.size() > 1 ) {
         throw phase2_access_return();
      }
      const auto& e = c.concat.front();
      switch( e.kind() ) {
         case entry_kind::value:
            throw std::string( "cannot index (across) value" );
         case entry_kind::reference:
            throw std::string( "cannot index (across) reference" );
         case entry_kind::array:
            if( e.get_array().array.size() > index ) {
               return phase2_access( *std::next( e.get_array().array.begin(), index ), suffix, down - 1 );
            }
            if( down >=0 ) {
               return nullptr;
            }
            throw std::string( "index out of range" );
         case entry_kind::object:
            throw std::string( "cannot index (across) object" );
      }
      assert( false );  // UNREACHABLE
   }

   [[nodiscard]] inline const concat* phase2_access( const concat& c, const key1_part& p, const key1& suffix, const int down )
   {
      switch( p.kind() ) {
         case key1_kind::star:
            throw pegtl::parse_error( "unable to access star", p.position );
         case key1_kind::name:
            return phase2_access_name( c, p.get_name(), suffix, down );
         case key1_kind::index:
            return phase2_access_index( c, p.get_index(), suffix, down );
         case key1_kind::append:
            throw pegtl::parse_error( "this should be impossible", p.position );
      }
      assert( false );  // UNREACHABLE
   }

   [[nodiscard]] inline const concat* phase2_access( const concat& c, const key1& suffix, const int down )
   {
      if( suffix.empty() ) {
         if( ( c.concat.size() < 2 ) && ( c.all_references() == 0 ) ) {
            return &c;
         }
         throw phase2_access_return();
      }
      return phase2_access( c, suffix.at( 0 ), pop_front( suffix ), down );
   }

   [[nodiscard]] inline const concat* phase2_access( const object& o, const key1& prefix, const key1& suffix )
   {
      assert( !suffix.empty() );

      try {
         for( std::size_t i = 0; i <= prefix.size(); ++i ) {
            const int down = int( prefix.size() ) - int( i );
            const key1 path = key1( prefix.begin(), prefix.end() - i ) + suffix;
            const auto j = o.object.find( path.front().get_name() );
            if( j != o.object.end() ) {
               if( const concat* c = phase2_access( j->second, pop_front( path ), down - 1 ) ) {
                  return c;
               }
            }
         }
         return nullptr;
      }
      catch( const phase2_access_return& /*unused*/ ) {
         return nullptr;
      }
   }

}  // namespace tao::config::internal

#endif
