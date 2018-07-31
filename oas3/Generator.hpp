/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi, <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#ifndef oatpp_swagger_oas3_Generator_hpp
#define oatpp_swagger_oas3_Generator_hpp

#include "oatpp-swagger/oas3/Model.hpp"

#include "oatpp/web/server/api/Endpoint.hpp"
#include "oatpp/core/collection/LinkedList.hpp"

#include <unordered_map>

namespace oatpp { namespace swagger { namespace oas3 {
  
class Generator {
public:
  typedef oatpp::web::server::api::Endpoint Endpoint;
  typedef oatpp::collection::LinkedList<std::shared_ptr<Endpoint>> Endpoints;
  
  template <class Value>
  using Fields = oatpp::data::mapping::type::ListMap<String, Value>;
  
  typedef Fields<PathItem::ObjectWrapper> Paths;
  
  typedef std::unordered_map<oatpp::String, const oatpp::data::mapping::type::Type*> UsedSchemas;
  
public:
  
  static Schema::ObjectWrapper generateSchemaForTypeList(const oatpp::data::mapping::type::Type* type, bool linkSchema, UsedSchemas& usedSchemas);
  static Schema::ObjectWrapper generateSchemaForTypeObject(const oatpp::data::mapping::type::Type* type, bool linkSchema, UsedSchemas& usedSchemas);
  static Schema::ObjectWrapper generateSchemaForType(const oatpp::data::mapping::type::Type* type, bool linkSchema, UsedSchemas& usedSchemas);

  static RequestBody::ObjectWrapper generateRequestBody(const Endpoint::Info& endpointInfo, bool linkSchema, UsedSchemas& usedSchemas);
  static Fields<OperationResponse::ObjectWrapper>::ObjectWrapper generateResponses(const Endpoint::Info& endpointInfo, bool linkSchema, UsedSchemas& usedSchemas);
  static void generatePathItemData(const std::shared_ptr<Endpoint>& endpoint, const PathItem::ObjectWrapper& pathItem, UsedSchemas& usedSchemas);
  
  /**
   *  UsedSchemas& usedSchemas is used to put Types of objects whos schema should be reused
   */
  static Paths::ObjectWrapper generatePaths(const std::shared_ptr<Endpoints>& endpoints, UsedSchemas& usedSchemas);
  
  static Components::ObjectWrapper generateComponents(const UsedSchemas& usedSchemas);
  
  static Document::ObjectWrapper generateDocument(const std::shared_ptr<oatpp::swagger::DocumentInfo>& docInfo, const std::shared_ptr<Endpoints>& endpoints);
  
  
  
};
  
}}}

#endif /* oatpp_swagger_oas3_Generator_hpp */