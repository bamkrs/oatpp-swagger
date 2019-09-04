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

#include "Generator.hpp"

#include "oatpp/core/utils/ConversionUtils.hpp"

namespace oatpp { namespace swagger { namespace oas3 {

const char* Generator::TAG = "oatpp-swagger::oas3::Generator";

Schema::ObjectWrapper Generator::generateSchemaForTypeObject(const oatpp::data::mapping::type::Type* type, bool linkSchema, UsedTypes& usedTypes) {

  OATPP_ASSERT(type && "[oatpp-swagger::oas3::Generator::generateSchemaForTypeObject()]: Error. Type should not be null.");

  auto result = Schema::createShared();
  if(linkSchema) {
  
    result->ref = oatpp::String("#/components/schemas/") + type->nameQualifier;
    usedTypes[type->nameQualifier] = type;
    return result;
  
  } else {
    
    result->type = "object";
    result->properties = result->properties->createShared();
    
    auto properties = type->properties;
    if(properties->getList().size() == 0) {
      type->creator(); // init type by creating first instance of that type
    }
    
    auto it = properties->getList().begin();
    while (it != properties->getList().end()) {
      auto p = *it ++;
      result->properties->put(p->name, generateSchemaForType(p->type, true, usedTypes));
    }
    
    return result;
  }
  
}
  
Schema::ObjectWrapper Generator::generateSchemaForTypeList(const oatpp::data::mapping::type::Type* type, bool linkSchema, UsedTypes& usedTypes) {

  OATPP_ASSERT(type && "[oatpp-swagger::oas3::Generator::generateSchemaForTypeList()]: Error. Type should not be null.");

  auto result = Schema::createShared();
  result->type = "array";
  result->items = generateSchemaForType(*type->params.begin(), linkSchema, usedTypes);
  return result;
}
  
Schema::ObjectWrapper Generator::generateSchemaForType(const oatpp::data::mapping::type::Type* type, bool linkSchema, UsedTypes& usedTypes) {

  OATPP_ASSERT(type && "[oatpp-swagger::oas3::Generator::generateSchemaForType()]: Error. Type should not be null.");

  auto typeName = type->name;
  if(typeName == oatpp::data::mapping::type::__class::String::CLASS_NAME){
    auto result = Schema::createShared();
    result->type = "string";
    return result;
  } else if(typeName == oatpp::data::mapping::type::__class::Int32::CLASS_NAME){
    auto result = Schema::createShared();
    result->type = "integer";
    result->format = "int32";
    return result;
  } else if(typeName == oatpp::data::mapping::type::__class::Int64::CLASS_NAME){
    auto result = Schema::createShared();
    result->type = "integer";
    result->format = "int64";
    return result;
  } else if(typeName == oatpp::data::mapping::type::__class::Float32::CLASS_NAME){
    auto result = Schema::createShared();
    result->type = "number";
    result->format = "float";
    return result;
  } else if(typeName == oatpp::data::mapping::type::__class::Float64::CLASS_NAME){
    auto result = Schema::createShared();
    result->type = "number";
    result->format = "double";
    return result;
  } else if(typeName == oatpp::data::mapping::type::__class::Boolean::CLASS_NAME){
    auto result = Schema::createShared();
    result->type = "boolean";
    return result;
  } else if(typeName == oatpp::data::mapping::type::__class::AbstractObject::CLASS_NAME){
    return generateSchemaForTypeObject(type, linkSchema, usedTypes);
  } else if(typeName == oatpp::data::mapping::type::__class::AbstractList::CLASS_NAME){
    return generateSchemaForTypeList(type, linkSchema, usedTypes);
  } else if(typeName == oatpp::data::mapping::type::__class::AbstractListMap::CLASS_NAME){
    // TODO
  } else {
    auto result = Schema::createShared();
    result->type = type->name;
    if(type->nameQualifier) {
      result->format = type->nameQualifier;
    }
    return result;
  }
  
  return Schema::createShared();
  
}

void Generator::addParamsToParametersList(const PathItemParameters::ObjectWrapper& paramsList,
                                          Endpoint::Info::Params& params,
                                          const oatpp::String& inType,
                                          UsedTypes& usedTypes)
{

  auto it = params.getOrder().begin();
  while (it != params.getOrder().end()) {
    auto param = params[*it++];
    auto parameter = PathItemParameter::createShared();
    parameter->in = inType;
    parameter->name = param.name;
    parameter->description = param.description;
    parameter->required = param.required;
    parameter->deprecated = param.deprecated;
    parameter->schema = generateSchemaForType(param.type, true, usedTypes);
    paramsList->pushBack(parameter);
  }

}

RequestBody::ObjectWrapper Generator::generateRequestBody(const Endpoint::Info& endpointInfo, bool linkSchema, UsedTypes& usedTypes) {

  if(endpointInfo.consumes.size() > 0) {

    auto body = RequestBody::createShared();
    body->description = "request body";
    body->content = body->content->createShared();
    
    auto it = endpointInfo.consumes.begin();
    while (it != endpointInfo.consumes.end()) {
      
      auto mediaType = MediaTypeObject::createShared();
      mediaType->schema = generateSchemaForType(it->schema, linkSchema, usedTypes);
      
      body->content->put(it->contentType, mediaType);
      
      it++;
    }

    return body;

  } else {
  
    if(endpointInfo.body.name != nullptr && endpointInfo.body.type != nullptr) {

      auto body = RequestBody::createShared();
      body->description = "request body";
      
      auto mediaType = MediaTypeObject::createShared();
      mediaType->schema = generateSchemaForType(endpointInfo.body.type, linkSchema, usedTypes);
      
      body->content = body->content->createShared();
      if(endpointInfo.bodyContentType != nullptr) {
        body->content->put(endpointInfo.bodyContentType, mediaType);
      } else {

        OATPP_ASSERT(endpointInfo.body.type && "[oatpp-swagger::oas3::Generator::generateRequestBody()]: Error. Type should not be null.");

        if(endpointInfo.body.type->name == oatpp::data::mapping::type::__class::AbstractObject::CLASS_NAME) {
          body->content->put("application/json", mediaType);
        } else if(endpointInfo.body.type->name == oatpp::data::mapping::type::__class::AbstractList::CLASS_NAME) {
          body->content->put("application/json", mediaType);
        } else if(endpointInfo.body.type->name == oatpp::data::mapping::type::__class::AbstractListMap::CLASS_NAME) {
          body->content->put("application/json", mediaType);
        } else {
          body->content->put("text/plain", mediaType);
        }
      }

      return body;

    }

  }
  
  return nullptr;

}

Generator::Fields<OperationResponse::ObjectWrapper>::ObjectWrapper Generator::generateResponses(const Endpoint::Info& endpointInfo, bool linkSchema, UsedTypes& usedTypes) {
  
  auto responses = Fields<OperationResponse::ObjectWrapper>::createShared();
  
  if(endpointInfo.responses.size() > 0) {
    
    auto it = endpointInfo.responses.begin();
    while (it != endpointInfo.responses.end()) {
      
      auto mediaType = MediaTypeObject::createShared();
      mediaType->schema = generateSchemaForType(it->second.schema, linkSchema, usedTypes);
      
      auto response = OperationResponse::createShared();
      response->description = it->first.description;
      response->content = response->content->createShared();
      response->content->put(it->second.contentType, mediaType);
      responses->put(oatpp::utils::conversion::int32ToStr(it->first.code), response);
      
      it++;
    }
    
  } else {
  
    auto mediaType = MediaTypeObject::createShared();
    mediaType->schema = generateSchemaForType(oatpp::String::Class::getType(), linkSchema, usedTypes);
  
    auto response = OperationResponse::createShared();
    response->description = "success";
    response->content = response->content->createShared();
    response->content->put("text/plain", mediaType);
    responses->put("200", response);
  
  }
  
  return responses;
    
}
  
void Generator::generatePathItemData(const std::shared_ptr<Endpoint>& endpoint, const PathItem::ObjectWrapper& pathItem, UsedTypes& usedTypes, UsedSSOs &usedSSOs) {
  
  auto info = endpoint->info();
  
  if(info) {
    
    auto operation = PathItemOperation::createShared();
    operation->operationId = info->name;
    operation->summary = info->summary;
    operation->description = info->description;
    
    if(oatpp::base::StrBuffer::equalsCI("get", info->method->c_str(), info->method->getSize())) {
      pathItem->operationGet = operation;
    } else if(oatpp::base::StrBuffer::equalsCI("put", info->method->c_str(), info->method->getSize())) {
      pathItem->operationPut = operation;
    } else if(oatpp::base::StrBuffer::equalsCI("post", info->method->c_str(), info->method->getSize())) {
      pathItem->operationPost = operation;
    } else if(oatpp::base::StrBuffer::equalsCI("delete", info->method->c_str(), info->method->getSize())) {
      pathItem->operationDelete = operation;
    } else if(oatpp::base::StrBuffer::equalsCI("options", info->method->c_str(), info->method->getSize())) {
      pathItem->operationOptions = operation;
    } else if(oatpp::base::StrBuffer::equalsCI("head", info->method->c_str(), info->method->getSize())) {
      pathItem->operationHead = operation;
    } else if(oatpp::base::StrBuffer::equalsCI("patch", info->method->c_str(), info->method->getSize())) {
      pathItem->operationPatch = operation;
    } else if(oatpp::base::StrBuffer::equalsCI("trace", info->method->c_str(), info->method->getSize())) {
      pathItem->operationTrace = operation;
    }
    
    operation->responses = generateResponses(*info, true, usedTypes);
    operation->requestBody = generateRequestBody(*info, true, usedTypes);

    if(!operation->parameters) {

      operation->parameters = operation->parameters->createShared();

      Endpoint::Info::Params filteredHeaders;
      if(!info->headers.getOrder().empty()) {
        for (const auto &header : info->headers.getOrder()) {
          // We don't want the Authorization header listed as Parameter. This should be done in ENDPOINT_INFO() { info->addSecurityRequirement( /* SSONAME */ ); }
          if (header != oatpp::web::protocol::http::Header::AUTHORIZATION) {
            filteredHeaders[header] = info->headers[header];
          }
        }
      }

      addParamsToParametersList(operation->parameters, filteredHeaders, "header", usedTypes);
      addParamsToParametersList(operation->parameters, info->pathParams, "path", usedTypes);
      addParamsToParametersList(operation->parameters, info->queryParams, "query", usedTypes);

    }

    if(!info->securityRequirements.empty()) {
      OATPP_ASSERT(info->authorization && "[oatpp-swagger::oas3::Generator::generatePathItemData()]: Error. Endpoint has security requirement but is no authorized endpoint.");
    }

    if(info->authorization) {
      OATPP_ASSERT(!info->securityRequirements.empty() && "[oatpp-swagger::oas3::Generator::generatePathItemData()]: Error. Authorized endpoint with no security requirements (info->addSecurityRequirement()) set.");

      if (!info->securityRequirements.empty()) {

        operation->security = operation->security->createShared();

        for (const auto &sec : info->securityRequirements) {

          usedSSOs[sec.first] = true;
          if (sec.second == nullptr) {

            // who ever came up to define "security" as an array of objects of array of strings
            auto fields = Fields<Components::List<String>::ObjectWrapper>::createShared();
            fields->put(sec.first, Components::List<String>::createShared());
            operation->security->pushBack(fields);

          } else {

            auto fields = Fields<Components::List<String>::ObjectWrapper>::createShared();
            auto sro = Components::List<String>::createShared();
            for (const auto &sr : *sec.second) {
              sro->pushBack(sr);
            }
            fields->put(sec.first, sro);
            operation->security->pushBack(fields);

          }
        }
      }
    }
  }
}
  
Generator::Paths::ObjectWrapper Generator::generatePaths(const std::shared_ptr<Endpoints>& endpoints, UsedTypes& usedTypes, UsedSSOs &usedSSOs) {
  
  auto result = Paths::createShared();
  
  auto curr = endpoints->getFirstNode();
  while (curr != nullptr) {
    auto endpoint = curr->getData();
    
    if(endpoint->info()) {
      oatpp::String path = endpoint->info()->path;
      if(path->getSize() == 0) {
        continue;
      }
      if(path->getData()[0] != '/') {
        path = "/" + path;
      }

      auto pathItem = result->get(path, nullptr);
      if(!pathItem) {
        pathItem = PathItem::createShared();
        result->put(path, pathItem);
      }

      generatePathItemData(endpoint, pathItem, usedTypes, usedSSOs);
    }
    
    curr = curr->getNext();
  }
  
  return result;
  
}
  
void Generator::decomposeObject(const oatpp::data::mapping::type::Type* type, UsedTypes& decomposedTypes) {

  OATPP_ASSERT(type && "[oatpp-swagger::oas3::Generator::decomposeObject()]: Error. Type should not be null.");

  auto schemaIt = decomposedTypes.find(type->nameQualifier);
  if(schemaIt != decomposedTypes.end()) {
    return;
  }
  
  decomposedTypes[type->nameQualifier] = type;
  
  auto properties = type->properties;
  if(properties->getList().size() == 0) {
    type->creator(); // init type by creating first instance of that type
  }
  
  auto it = properties->getList().begin();
  while (it != properties->getList().end()) {
    auto p = *it ++;
    decomposeType(p->type, decomposedTypes);
  }
}

void Generator::decomposeList(const oatpp::data::mapping::type::Type* type, UsedTypes& decomposedTypes) {
  OATPP_ASSERT(type && "[oatpp-swagger::oas3::Generator::decomposeList()]: Error. Type should not be null.");
  decomposeType(*type->params.begin(), decomposedTypes);
}

void Generator::decomposeMap(const oatpp::data::mapping::type::Type* type, UsedTypes& decomposedTypes) {
  // TODO
}
  
void Generator::decomposeType(const oatpp::data::mapping::type::Type* type, UsedTypes& decomposedTypes) {
  OATPP_ASSERT(type && "[oatpp-swagger::oas3::Generator::decomposeType()]: Error. Type should not be null.");
  auto typeName = type->name;
  if(typeName == oatpp::data::mapping::type::__class::AbstractObject::CLASS_NAME){
    decomposeObject(type, decomposedTypes);
  } else if(typeName == oatpp::data::mapping::type::__class::AbstractList::CLASS_NAME){
    decomposeList(type, decomposedTypes);
  } else if(typeName == oatpp::data::mapping::type::__class::AbstractListMap::CLASS_NAME){
    decomposeMap(type, decomposedTypes);
  }
}
  
Generator::UsedTypes Generator::decomposeTypes(UsedTypes& usedTypes) {
  
  UsedTypes result; // decomposed schemas
  
  auto it = usedTypes.begin();
  while (it != usedTypes.end()) {
    decomposeType(it->second, result);
    result[it->first] = it->second;
    it ++;
  }
  
  return result;
  
}
  
Components::ObjectWrapper Generator::generateComponents(const UsedTypes &decomposedTypes,
                                                        const std::shared_ptr<std::unordered_map<oatpp::String,std::shared_ptr<oatpp::swagger::SecuritySchemeObject>>> &ssos,
                                                        UsedSSOs &usedSSOs) {
  
  auto result = Components::createShared();
  result->schemas = result->schemas->createShared();
  
  auto it = decomposedTypes.begin();
  while (it != decomposedTypes.end()) {
    UsedTypes schemas; ///< dummy
    result->schemas->put(it->first, generateSchemaForType(it->second, false, schemas));
    it ++;
  }

  if(ssos) {
    result->securitySchemes = result->securitySchemes->createShared();
    for (const auto &sso : usedSSOs) {
        OATPP_ASSERT(ssos->find(sso.first) != ssos->end() && "[oatpp-swagger::oas3::Generator::generateComponents()]: Error. Requested unknown security requirement.");
        result->securitySchemes->put(sso.first, generateSSO(ssos->at(sso.first)));
    }
  }

  return result;
  
}
  
Document::ObjectWrapper Generator::generateDocument(const std::shared_ptr<oatpp::swagger::DocumentInfo>& docInfo, const std::shared_ptr<Endpoints>& endpoints) {
  
  auto document = oas3::Document::createShared();
  document->info = Info::createFromBaseModel(docInfo->header);
  
  if(docInfo->servers) {
    document->servers = document->servers->createShared();

    for(const auto &it : *docInfo->servers) {
      document->servers->pushBack(Server::createFromBaseModel(it));
    }

  }
  
  UsedTypes usedTypes;
  UsedSSOs usedSSOs;
  document->paths = generatePaths(endpoints, usedTypes, usedSSOs);
  auto decomposedTypes = decomposeTypes(usedTypes);
  document->components = generateComponents(decomposedTypes, docInfo->ssos, usedSSOs);

  return document;
  
}

SecuritySchemeObject::ObjectWrapper Generator::generateSSO(const std::shared_ptr<oatpp::swagger::SecuritySchemeObject> &sso) {
  auto oassso = oatpp::swagger::oas3::SecuritySchemeObject::createShared();

  oassso->type = sso->type;
  oassso->description = sso->description;
  oassso->openIdConnectUrl = sso->openIdConnectUrl;
  oassso->in = sso->in;
  oassso->bearerFormat = sso->bearerFormat;
  oassso->name = sso->name;
  oassso->scheme = sso->scheme;

  if(sso->flows) {
    oassso->flows = oassso->flows->createShared();
    if(sso->flows->implicit) {
      oassso->flows->implicit = oassso->flows->implicit->createShared();
      oassso->flows->implicit->tokenUrl = sso->flows->implicit->tokenUrl;
      oassso->flows->implicit->refreshUrl = sso->flows->implicit->refreshUrl;
      oassso->flows->implicit->authorizationUrl = sso->flows->implicit->authorizationUrl;
      if(sso->flows->implicit->scopes) {
        oassso->flows->implicit->scopes->createShared();
        for(const auto &scope : *sso->flows->implicit->scopes) {
          oassso->flows->implicit->scopes->put(scope.first, scope.second);
        }
      }
    }
    if(sso->flows->password) {
      oassso->flows->password = oassso->flows->password->createShared();
      oassso->flows->password->tokenUrl = sso->flows->password->tokenUrl;
      oassso->flows->password->refreshUrl = sso->flows->password->refreshUrl;
      oassso->flows->password->authorizationUrl = sso->flows->password->authorizationUrl;
      if(sso->flows->password->scopes) {
        oassso->flows->password->scopes->createShared();
        for(const auto &scope : *sso->flows->password->scopes) {
          oassso->flows->password->scopes->put(scope.first, scope.second);
        }
      }
    }
    if(sso->flows->clientCredentials) {
      oassso->flows->clientCredentials = oassso->flows->clientCredentials->createShared();
      oassso->flows->clientCredentials->tokenUrl = sso->flows->clientCredentials->tokenUrl;
      oassso->flows->clientCredentials->refreshUrl = sso->flows->clientCredentials->refreshUrl;
      oassso->flows->clientCredentials->authorizationUrl = sso->flows->clientCredentials->authorizationUrl;
      if(sso->flows->clientCredentials->scopes) {
        oassso->flows->clientCredentials->scopes->createShared();
        for(const auto &scope : *sso->flows->clientCredentials->scopes) {
          oassso->flows->clientCredentials->scopes->put(scope.first, scope.second);
        }
      }
    }
    if(sso->flows->authorizationCode) {
      oassso->flows->authorizationCode = oassso->flows->authorizationCode->createShared();
      oassso->flows->authorizationCode->tokenUrl = sso->flows->authorizationCode->tokenUrl;
      oassso->flows->authorizationCode->refreshUrl = sso->flows->authorizationCode->refreshUrl;
      oassso->flows->authorizationCode->authorizationUrl = sso->flows->authorizationCode->authorizationUrl;
      if(sso->flows->authorizationCode->scopes) {
        oassso->flows->authorizationCode->scopes->createShared();
        for(const auto &scope : *sso->flows->authorizationCode->scopes) {
          oassso->flows->authorizationCode->scopes->put(scope.first, scope.second);
        }
      }
    }
  }
  return oassso;
}

}}}
