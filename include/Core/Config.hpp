#pragma once

#include <functional>

#include "Core/Core.hpp"

// Config files are essentialy .ini files, but they use .cfg extension to allow for potentional future format modifications. 

struct ConfigKeyValueNode
{
  bool isKey(const char* inKey) const;

  bool toBool() const;
  int64 toInt() const;
  float toFloat() const;
  double toDouble() const;

  char* key;
  char* value;
  const char* sectionPath;
  int64 keyLength;
  int64 valueLength;
  int64 sectionPathLength;
};
using ConfigKeyValueNodeCallback = std::function<bool(const ConfigKeyValueNode&)>;

struct ConfigSectionNode
{
  // TODO: Find out which fields are worth to keep. Are data and sectionPath same in this case?
  char* data;
  char* value;
  const char* sectionPath;
  int64 dataLength;
  int64 valueLength;
  int64 sectionPathLength;
};
using ConfigSectionNodeCallback = std::function<bool(const ConfigSectionNode&)>;


/**
 * Parses a config from a memory buffer. Calls callbacks for both sections and key/value pairs.
 * @param data buffer with the loaded config, WARNING: will be modified by the parser, data[dataLength - 1] will be set to 0, so it shouldn't contain any values (can be empty space, new line etc).
 * @param sectionNodeCallback called for each section node, return false to continue parsing, true to stop parsing.
 * @param keyValueNodeCallback called for each key/value pair node, return false to continue parsing, true to stop parsing.
 */
bool tryParseConfig(char* data, int64 dataLength, const ConfigSectionNodeCallback& sectionNodeCallback, const ConfigKeyValueNodeCallback& keyValueNodeCallback);
/**
 * Overload which ignores sections. 
 */ 
bool tryParseConfig(char* data, int64 dataLength, const ConfigKeyValueNodeCallback& nodeCallback);
