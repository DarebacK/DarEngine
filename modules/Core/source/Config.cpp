#define DAR_MODULE_NAME "Config"

#include "Core/Config.hpp"

#include "external/libconfini/confini.h"

bool ConfigKeyValueNode::toBool() const
{
  int retVal = ini_get_bool(value, -1);
  if (retVal == -1)
  {
    logError("Failed to parse value %s with key %s in section %s as bool, returning false.", value, key, sectionPath);
    return false;
  }

  return retVal;
}

int64 ConfigKeyValueNode::toInt() const
{
  return ini_get_llint(value);
}

float ConfigKeyValueNode::toFloat() const
{
  return ini_get_double(value);
}

double ConfigKeyValueNode::toDouble() const
{
  return ini_get_double(value);
}

static ConfigKeyValueNode toConfigKeyValueNode(IniDispatch* dispatch)
{
  ConfigKeyValueNode node;
  node.key = dispatch->data;
  node.value = dispatch->value;
  node.sectionPath = dispatch->append_to;
  node.dataLength = dispatch->d_len;
  node.valueLength = dispatch->v_len;
  node.sectionPathLength = dispatch->at_len;
  return node;
}

static bool libconfiniParse(char* data, int64 dataLength, IniDispHandler callback, void* userData)
{
  switch (strip_ini_cache(data, size_t(dataLength), INI_DEFAULT_FORMAT, nullptr, callback, userData))
  {
    case CONFINI_SUCCESS:
    case CONFINI_IINTR:
    case CONFINI_FEINTR:
      return true;

    default:
      return false;
  }
}

static int parseKeyValueCallback(IniDispatch* dispatch, void* userData)
{
  if (dispatch->type == INI_KEY)
  {
    const ConfigKeyValueNodeCallback& nodeCallback = *(ConfigKeyValueNodeCallback*)userData;
    return nodeCallback(toConfigKeyValueNode(dispatch));
  }

  return false;
}

bool tryParseConfig(char* data, int64 dataLength, const ConfigKeyValueNodeCallback& nodeCallback)
{
  if (!data || dataLength <= 0)
  {
    return false;
  }

  return libconfiniParse(data, dataLength, parseKeyValueCallback, (void*)&nodeCallback);
}

struct SectionOrKeyValueCallbackContext
{
  const ConfigSectionNodeCallback& sectionNodeCallback;
  const ConfigKeyValueNodeCallback& keyValueNodeCallback;
};
static int parseSectionOrKeyValueCallback(IniDispatch* dispatch, void* userData)
{
  const SectionOrKeyValueCallbackContext& context = *(SectionOrKeyValueCallbackContext*)userData;

  switch (dispatch->type)
  {
    case INI_KEY:
    {
      return context.keyValueNodeCallback(toConfigKeyValueNode(dispatch));
    }

    case INI_SECTION:
    {
      ConfigSectionNode node;
      node.value = dispatch->value;
      node.sectionPath = dispatch->append_to;
      node.dataLength = dispatch->d_len;
      node.valueLength = dispatch->v_len;
      node.sectionPathLength = dispatch->at_len;

      return context.sectionNodeCallback(node);
    }

    default:
      return false;
  }
}

bool tryParseConfig(char* data, int64 dataLength, const ConfigSectionNodeCallback& sectionNodeCallback, const ConfigKeyValueNodeCallback& keyValueNodeCallback)
{
  if (!data || dataLength <= 0)
  {
    return false;
  }

  SectionOrKeyValueCallbackContext context{ sectionNodeCallback, keyValueNodeCallback };
  return libconfiniParse(data, dataLength, parseSectionOrKeyValueCallback, (void*)&context);
}
