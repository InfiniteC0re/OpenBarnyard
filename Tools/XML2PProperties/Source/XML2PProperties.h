#pragma once
#include <Toshi/Toshi.h>
#include <Toshi/TPString8.h>

#include <Plugins/PTRB.h>
#include <Plugins/PPropertyParser/PPropertiesWriter.h>

#include <ToshiTools/json.hpp>
#include <ToshiTools/tinyxml2.h>

namespace XML2PProperties
{

void JsonToProperties( nlohmann::json& a_rJSON, PBProperties& a_rProperties );
void XmlToProperties( tinyxml2::XMLElement* a_pRootElement, PBProperties& a_rProperties );

tinyxml2::XMLElement* CreatePropertiesXMLTemplate( tinyxml2::XMLDocument& a_rOutDoc, TBOOL a_bCompressed );
void                  PropertiesToXml( tinyxml2::XMLElement* a_pRootElement, const PBProperties* a_pProperties );
TBOOL                 PropertiesToXml( tinyxml2::XMLDocument& a_rOutDoc, const TCHAR* a_pchFileName );

}
