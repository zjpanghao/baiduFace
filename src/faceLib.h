#ifndef INCLUDE_FACE_LIB_H
#define INCLUDE_FACE_LIB_H
#include "zdb/zdb.h"
#include "faceEntity.h"
namespace kface {
template <class T>
class FaceLibMapper : public RowMapper<T> {
 public:
   int mapRow(ResultSet_T r,
       int i,
       T &t) {
      this->getString(r, "group_id",
          t.groupId);
      this->getString(r,"user_id", t.userId);
      this->getString(r,"user_name", t.userName);
      t.image = std::make_shared<ImageFace>();
      this->getString(r, "face_token",t.image->faceToken);
      std::string feature;
      this->getString(r, "feature", feature);
      std::vector<unsigned char> vec;
      Base64::getBase64().decode(feature, vec);
      if (vec.size() != FACE_VEC_SIZE  *sizeof(float)) {
        return -1;
      }
      t.image->feature.assign((float*)&vec[0], (float*)(&vec[FACE_VEC_SIZE* sizeof(float)]));
   }
};

} // namespace kface
#endif
