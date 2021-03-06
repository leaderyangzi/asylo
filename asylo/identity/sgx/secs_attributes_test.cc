/*
 *
 * Copyright 2017 Asylo authors
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
 */

#include "asylo/identity/sgx/secs_attributes.h"

#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include "absl/strings/str_cat.h"
#include "asylo/identity/util/bit_vector_128.pb.h"
#include "asylo/identity/util/trivial_object_util.h"

namespace asylo {
namespace sgx {
namespace {

// A test fixture is used to ensure naming consistency, maintaining common
// set of constants, and future extensibility.
class SecsAttributesTest : public ::testing::Test {
 protected:
  SecsAttributesTest() {}

  // Independently define all the attribute bits so that any error
  // introduced in this part of the header file is caught.
  const std::vector<SecsAttributeBit> attributes_ = {
      static_cast<SecsAttributeBit>(0),  // FLAG_ATTRIBUTE_INIT
      static_cast<SecsAttributeBit>(1),  // FLAG_ATTRIBUTE_DEBUG
      static_cast<SecsAttributeBit>(2),  // FLAG_ATTRIBUTE_MODE64BIT
                                         // Unused bit
      static_cast<SecsAttributeBit>(4),  // FLAG_ATTRIBUTE_PROVISIONKEY
      static_cast<SecsAttributeBit>(5),  // FLAG_ATTRIBUTE_INITTOKENKEY

      static_cast<SecsAttributeBit>(64),  // XFRM_ATTRIBUTE_FPU,
      static_cast<SecsAttributeBit>(65),  // XFRM_ATTRIBUTE_SSE,
      static_cast<SecsAttributeBit>(66),  // XFRM_ATTRIBUTE_AVX,
      static_cast<SecsAttributeBit>(67),  // XFRM_ATTRIBUTE_BNDREG,
      static_cast<SecsAttributeBit>(68),  // XFRM_ATTRIBUTE_BNDCSR,
      static_cast<SecsAttributeBit>(69),  // XFRM_ATTRIBUTE_OPMASK,
      static_cast<SecsAttributeBit>(70),  // XFRM_ATTRIBUTE_ZMM_HI256,
      static_cast<SecsAttributeBit>(71),  // XFRM_ATTRIBUTE_HI16_ZMM,
                                          // Unused bit
      static_cast<SecsAttributeBit>(73)   // XFRM_ATTRIBUTE_PKRU
  };
  const std::vector<std::string> attribute_names_ = {
      "INIT",   "DEBUG",     "MODE64BIT", "PROVISIONKEY", "INITTOKENKEY",
      "FPU",    "SSE",       "AVX",       "BNDREG",       "BNDCSR",
      "OPMASK", "ZMM_HI256", "HI16_ZMM",  "PKRU"};
  const std::vector<SecsAttributeSet> attribute_sets_ = {
      {0x1, 0x0},
      {0x2, 0x0},
      {0x4, 0x0},
      // Unused bit
      {0x10, 0x0},
      {0x20, 0x0},

      {0x0, 0x1},
      {0x0, 0x2},
      {0x0, 0x4},
      {0x0, 0x8},
      {0x0, 0x10},
      {0x0, 0x20},
      {0x0, 0x40},
      {0x0, 0x80},
      // Unused bit
      {0x0, 0x200},
  };
  const SecsAttributeSet all_attributes_ = {0x37, 0x2FF};
  const SecsAttributeBit bad_attribute_ = static_cast<SecsAttributeBit>(129);
};

#define EXPECT_LOG(TYPE, MESSAGE)
#if 0
#define EXPECT_LOG(TYPE, MESSAGE)                                            \
  EXPECT_CALL(mock_log_, Log(TYPE, ::testing::_, ::testing::StrEq(MESSAGE))) \
      .Times(1)
#endif

// Verify the correctness of ClearSecsAttributeSet.
TEST_F(SecsAttributesTest, ClearSecsAttributeSet) {
  for (SecsAttributeSet set : attribute_sets_) {
    ClearSecsAttributeSet(&set);
    EXPECT_EQ(set.flags, 0);
    EXPECT_EQ(set.xfrm, 0);
  }

  SecsAttributeSet set;
  set = all_attributes_;
  ClearSecsAttributeSet(&set);
  EXPECT_EQ(set.flags, 0);
  EXPECT_EQ(set.xfrm, 0);
}

// Verify the correctness of the equality operator.
TEST_F(SecsAttributesTest, Equality) {
  for (int i = 0; i < attributes_.size(); i++) {
    for (int j = 0; j < attributes_.size(); j++) {
      EXPECT_EQ((attributes_[i] == attributes_[j]), (i == j));
    }
  }
}

// Verify the correctness of the inequality operator.
TEST_F(SecsAttributesTest, Inequality) {
  for (int i = 0; i < attributes_.size(); i++) {
    for (int j = 0; j < attributes_.size(); j++) {
      EXPECT_EQ((attributes_[i] != attributes_[j]), (i != j));
    }
  }
}

// Verify the correctness of bit-wise OR operator.
TEST_F(SecsAttributesTest, BitwiseOr) {
  SecsAttributeSet result;

  for (const SecsAttributeSet &set : attribute_sets_) {
    result = set | set;
    EXPECT_EQ(result.flags, set.flags);
    EXPECT_EQ(result.xfrm, set.xfrm);
  }

  result = TrivialZeroObject<SecsAttributeSet>();
  for (const SecsAttributeSet &set : attribute_sets_) {
    result = result | set;
  }
  EXPECT_EQ(result.flags, all_attributes_.flags);
  EXPECT_EQ(result.xfrm, all_attributes_.xfrm);

  SecsAttributeSet cleared_set = TrivialZeroObject<SecsAttributeSet>();
  result = result | cleared_set;
  EXPECT_EQ(result.flags, all_attributes_.flags);
  EXPECT_EQ(result.xfrm, all_attributes_.xfrm);
}

// Verify the correctness of bit-wise AND operator.
TEST_F(SecsAttributesTest, BitwiseAnd) {
  for (const SecsAttributeSet &set : attribute_sets_) {
    SecsAttributeSet result = all_attributes_ & set;
    EXPECT_EQ(result.flags, set.flags);
    EXPECT_EQ(result.xfrm, set.xfrm);
  }

  SecsAttributeSet cleared_set = TrivialZeroObject<SecsAttributeSet>();
  for (const SecsAttributeSet &set : attribute_sets_) {
    SecsAttributeSet result = cleared_set & set;
    EXPECT_EQ(result.flags, 0);
    EXPECT_EQ(result.xfrm, 0);
  }
}

// Verify the correctness of bit-wise negation operator.
TEST_F(SecsAttributesTest, BitwiseNegation) {
  SecsAttributeSet zeros = TrivialZeroObject<SecsAttributeSet>();
  SecsAttributeSet ones = TrivialOnesObject<SecsAttributeSet>();

  for (const SecsAttributeSet &set : attribute_sets_) {
    EXPECT_EQ(set & ~set, zeros);
    EXPECT_EQ(set | ~set, ones);
  }
}

// Verify the correctness of conversion from attribute list to attribute set.
TEST_F(SecsAttributesTest, ListToSet) {
  for (int i = 0; i < attributes_.size(); i++) {
    std::vector<SecsAttributeBit> v{attributes_[i]};

    SecsAttributeSet set;
    EXPECT_TRUE(ConvertSecsAttributeRepresentation(v, &set));
    EXPECT_EQ(set.flags, attribute_sets_[i].flags);
    EXPECT_EQ(set.xfrm, attribute_sets_[i].xfrm);
  }

  SecsAttributeSet set;
  EXPECT_TRUE(ConvertSecsAttributeRepresentation(attributes_, &set));
  EXPECT_EQ(set.flags, all_attributes_.flags);
  EXPECT_EQ(set.xfrm, all_attributes_.xfrm);
}

// Verify error condition for conversion from attribute list to attribute set.
TEST_F(SecsAttributesTest, ListToSetError) {
  std::vector<SecsAttributeBit> v{bad_attribute_};
  std::string str =
      absl::StrCat("SecsAttributeBit specifies a bit position ",
                   static_cast<size_t>(bad_attribute_),
                   " that is larger than the max allowed value of 127");
  EXPECT_LOG(ERROR, str);
  SecsAttributeSet set;
  EXPECT_FALSE(ConvertSecsAttributeRepresentation(v, &set));
}

// Verify the correctness of conversion from AttributeSet to attribute list.
TEST_F(SecsAttributesTest, SetToList) {
  std::vector<SecsAttributeBit> list;
  for (int i = 0; i < attribute_sets_.size(); i++) {
    EXPECT_TRUE(ConvertSecsAttributeRepresentation(attribute_sets_[i], &list));
    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list[0], attributes_[i]);
  }

  EXPECT_TRUE(ConvertSecsAttributeRepresentation(all_attributes_, &list));
  EXPECT_EQ(list.size(), attributes_.size());
  for (int i = 0; i < list.size(); i++) {
    EXPECT_EQ(list[i], attributes_[i]);
  }
}

// Verify the correctness of conversion from attribute list to BitVector128.
TEST_F(SecsAttributesTest, ListToBitVector) {
  for (int i = 0; i < attributes_.size(); i++) {
    std::vector<SecsAttributeBit> v{attributes_[i]};

    BitVector128 bit_vector;
    EXPECT_TRUE(ConvertSecsAttributeRepresentation(v, &bit_vector));
    EXPECT_EQ(bit_vector.low(), attribute_sets_[i].flags);
    EXPECT_EQ(bit_vector.high(), attribute_sets_[i].xfrm);
  }

  BitVector128 bit_vector;
  EXPECT_TRUE(ConvertSecsAttributeRepresentation(all_attributes_, &bit_vector));
  EXPECT_EQ(bit_vector.low(), all_attributes_.flags);
  EXPECT_EQ(bit_vector.high(), all_attributes_.xfrm);
}

// Verify error handling for conversion from attribute list to BitVector128.
TEST_F(SecsAttributesTest, ListToBitVectorError) {
  std::vector<SecsAttributeBit> v{bad_attribute_};
  std::string str =
      absl::StrCat("SecsAttributeBit specifies a bit position ",
                   static_cast<size_t>(bad_attribute_),
                   " that is larger than the max allowed value of 127");
  EXPECT_LOG(ERROR, str);
  BitVector128 bit_vector;
  EXPECT_FALSE(ConvertSecsAttributeRepresentation(v, &bit_vector));
}

// Verify the correctness of conversion from BitVector128 to attribute list.
TEST_F(SecsAttributesTest, BitVectorToList) {
  std::vector<SecsAttributeBit> list;
  BitVector128 bit_vector;
  for (int i = 0; i < attribute_sets_.size(); i++) {
    bit_vector.set_low(attribute_sets_[i].flags);
    bit_vector.set_high(attribute_sets_[i].xfrm);
    EXPECT_TRUE(ConvertSecsAttributeRepresentation(bit_vector, &list));
    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list[0], attributes_[i]);
  }

  bit_vector.set_low(all_attributes_.flags);
  bit_vector.set_high(all_attributes_.xfrm);
  EXPECT_TRUE(ConvertSecsAttributeRepresentation(bit_vector, &list));
  EXPECT_EQ(list.size(), attributes_.size());
  for (int i = 0; i < list.size(); i++) {
    EXPECT_EQ(list[i], attributes_[i]);
  }
}

// Verify the correctness of conversion from AttributeSet to BitVector128.
TEST_F(SecsAttributesTest, SetToBitVector) {
  BitVector128 bit_vector;
  for (int i = 0; i < attribute_sets_.size(); i++) {
    BitVector128 bit_vector;
    EXPECT_TRUE(
        ConvertSecsAttributeRepresentation(attribute_sets_[i], &bit_vector));
    EXPECT_EQ(bit_vector.low(), attribute_sets_[i].flags);
    EXPECT_EQ(bit_vector.high(), attribute_sets_[i].xfrm);
  }

  EXPECT_TRUE(ConvertSecsAttributeRepresentation(attributes_, &bit_vector));
  EXPECT_EQ(bit_vector.low(), all_attributes_.flags);
  EXPECT_EQ(bit_vector.high(), all_attributes_.xfrm);
}

// Verify the correctness of conversion from BitVector128 to AttributeSet.
TEST_F(SecsAttributesTest, BitVectorToSet) {
  std::vector<SecsAttributeBit> list;
  BitVector128 bit_vector;
  SecsAttributeSet attribute_set;

  for (int i = 0; i < attribute_sets_.size(); i++) {
    bit_vector.set_low(attribute_sets_[i].flags);
    bit_vector.set_high(attribute_sets_[i].xfrm);
    EXPECT_TRUE(ConvertSecsAttributeRepresentation(bit_vector, &attribute_set));
    EXPECT_EQ(bit_vector.low(), attribute_set.flags);
    EXPECT_EQ(bit_vector.high(), attribute_set.xfrm);
  }

  bit_vector.set_low(all_attributes_.flags);
  bit_vector.set_high(all_attributes_.xfrm);
  EXPECT_TRUE(ConvertSecsAttributeRepresentation(bit_vector, &attribute_set));
  EXPECT_EQ(bit_vector.low(), all_attributes_.flags);
  EXPECT_EQ(bit_vector.high(), all_attributes_.xfrm);
}

// Verify the correctness of TestAttribute on a set.
TEST_F(SecsAttributesTest, TestAttributeSet) {
  for (int i = 0; i < attribute_sets_.size(); i++) {
    for (int j = 0; j < attributes_.size(); j++) {
      EXPECT_EQ(TestAttribute(attributes_[j], attribute_sets_[i]), (i == j));
    }
  }
  for (int j = 0; j < attributes_.size(); j++) {
    EXPECT_TRUE(TestAttribute(attributes_[j], all_attributes_));
  }
}

// Verify the error-handling in TestAttribute on a set.
TEST_F(SecsAttributesTest, TestAttributeSetError) {
  std::string str =
      absl::StrCat("SecsAttributeBit specifies a bit position ",
                   static_cast<size_t>(bad_attribute_),
                   " that is larger than the max allowed value of 127");

  EXPECT_LOG(INFO, str);

  EXPECT_FALSE(TestAttribute(bad_attribute_, all_attributes_));
}

// Verify the correctness of TestAttribute on a BitVector.
TEST_F(SecsAttributesTest, TestAttributeBitVector) {
  BitVector128 bit_vector;
  for (int i = 0; i < attribute_sets_.size(); i++) {
    EXPECT_TRUE(
        ConvertSecsAttributeRepresentation(attribute_sets_[i], &bit_vector));
    for (int j = 0; j < attributes_.size(); j++) {
      EXPECT_EQ(TestAttribute(attributes_[j], bit_vector), (i == j));
    }
  }
  EXPECT_TRUE(ConvertSecsAttributeRepresentation(all_attributes_, &bit_vector));
  for (int j = 0; j < attributes_.size(); j++) {
    EXPECT_TRUE(TestAttribute(attributes_[j], bit_vector));
  }
}

// Verify the error-handling in TestAttribute on a set.
TEST_F(SecsAttributesTest, TestAttributeBitVectorError) {
  std::string str =
      absl::StrCat("SecsAttributeBit specifies a bit position ",
                   static_cast<size_t>(bad_attribute_),
                   " that is larger than the max allowed value of 127");

  EXPECT_LOG(INFO, str);

  BitVector128 bit_vector;
  EXPECT_TRUE(ConvertSecsAttributeRepresentation(all_attributes_, &bit_vector));
  EXPECT_FALSE(TestAttribute(bad_attribute_, bit_vector));
}

// Verify the correctness of GetPrintableAttributeList on an attribute list.
TEST_F(SecsAttributesTest, GetPrintableAttributeListFromList) {
  std::vector<std::string> printable_list;

  for (int i = 0; i < attribute_sets_.size(); i++) {
    std::vector<SecsAttributeBit> attribute_bit_list = {attributes_[i]};
    GetPrintableAttributeList(attribute_bit_list, &printable_list);
    EXPECT_EQ(printable_list.size(), 1);
    EXPECT_EQ(printable_list[0], attribute_names_[i]);
  }

  GetPrintableAttributeList(attributes_, &printable_list);
  EXPECT_EQ(printable_list.size(), attributes_.size());
  for (int i = 0; i < printable_list.size(); i++) {
    EXPECT_EQ(printable_list[i], attribute_names_[i]);
  }
}

// Verify the correctness of GetPrintableAttributeList on an attribute set.
TEST_F(SecsAttributesTest, GetPrintableAttributeListFromSet) {
  std::vector<std::string> printable_list;

  for (int i = 0; i < attribute_sets_.size(); i++) {
    GetPrintableAttributeList(attribute_sets_[i], &printable_list);
    EXPECT_EQ(printable_list.size(), 1);
    EXPECT_EQ(printable_list[0], attribute_names_[i]);
  }

  GetPrintableAttributeList(all_attributes_, &printable_list);
  EXPECT_EQ(printable_list.size(), attributes_.size());
  for (int i = 0; i < printable_list.size(); i++) {
    EXPECT_EQ(printable_list[i], attribute_names_[i]);
  }
}

// Verify the correctness of GetPrintableAttributeList on a bit vector.
TEST_F(SecsAttributesTest, GetPrintableAttributeListFromBitVector) {
  std::vector<std::string> printable_list;
  BitVector128 bit_vector;

  for (int i = 0; i < attribute_sets_.size(); i++) {
    EXPECT_TRUE(
        ConvertSecsAttributeRepresentation(attribute_sets_[i], &bit_vector));
    GetPrintableAttributeList(bit_vector, &printable_list);
    EXPECT_EQ(printable_list.size(), 1);
    EXPECT_EQ(printable_list[0], attribute_names_[i]);
  }

  EXPECT_TRUE(ConvertSecsAttributeRepresentation(all_attributes_, &bit_vector));
  GetPrintableAttributeList(bit_vector, &printable_list);
  EXPECT_EQ(printable_list.size(), attributes_.size());
  for (int i = 0; i < printable_list.size(); i++) {
    EXPECT_EQ(printable_list[i], attribute_names_[i]);
  }
}

}  // namespace
}  // namespace sgx
}  // namespace asylo
