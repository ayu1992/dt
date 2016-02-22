// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: dump.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "dump.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/port.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace motionClustering {

namespace {

const ::google::protobuf::Descriptor* Trajectory_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  Trajectory_reflection_ = NULL;
const ::google::protobuf::Descriptor* VideoInstance_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  VideoInstance_reflection_ = NULL;
const ::google::protobuf::Descriptor* VideoList_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  VideoList_reflection_ = NULL;

}  // namespace


void protobuf_AssignDesc_dump_2eproto() {
  protobuf_AddDesc_dump_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "dump.proto");
  GOOGLE_CHECK(file != NULL);
  Trajectory_descriptor_ = file->message_type(0);
  static const int Trajectory_offsets_[1] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Trajectory, normalizedpoints_),
  };
  Trajectory_reflection_ =
    ::google::protobuf::internal::GeneratedMessageReflection::NewGeneratedMessageReflection(
      Trajectory_descriptor_,
      Trajectory::default_instance_,
      Trajectory_offsets_,
      -1,
      -1,
      -1,
      sizeof(Trajectory),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Trajectory, _internal_metadata_),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Trajectory, _is_default_instance_));
  VideoInstance_descriptor_ = file->message_type(1);
  static const int VideoInstance_offsets_[4] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(VideoInstance, actionlabel_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(VideoInstance, videoindex_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(VideoInstance, numclusters_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(VideoInstance, tracks_),
  };
  VideoInstance_reflection_ =
    ::google::protobuf::internal::GeneratedMessageReflection::NewGeneratedMessageReflection(
      VideoInstance_descriptor_,
      VideoInstance::default_instance_,
      VideoInstance_offsets_,
      -1,
      -1,
      -1,
      sizeof(VideoInstance),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(VideoInstance, _internal_metadata_),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(VideoInstance, _is_default_instance_));
  VideoList_descriptor_ = file->message_type(2);
  static const int VideoList_offsets_[1] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(VideoList, videos_),
  };
  VideoList_reflection_ =
    ::google::protobuf::internal::GeneratedMessageReflection::NewGeneratedMessageReflection(
      VideoList_descriptor_,
      VideoList::default_instance_,
      VideoList_offsets_,
      -1,
      -1,
      -1,
      sizeof(VideoList),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(VideoList, _internal_metadata_),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(VideoList, _is_default_instance_));
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_dump_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
      Trajectory_descriptor_, &Trajectory::default_instance());
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
      VideoInstance_descriptor_, &VideoInstance::default_instance());
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
      VideoList_descriptor_, &VideoList::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_dump_2eproto() {
  delete Trajectory::default_instance_;
  delete Trajectory_reflection_;
  delete VideoInstance::default_instance_;
  delete VideoInstance_reflection_;
  delete VideoList::default_instance_;
  delete VideoList_reflection_;
}

void protobuf_AddDesc_dump_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\ndump.proto\022\020motionClustering\"&\n\nTrajec"
    "tory\022\030\n\020normalizedPoints\030\001 \003(\002\"{\n\rVideoI"
    "nstance\022\023\n\013actionLabel\030\001 \001(\t\022\022\n\nvideoInd"
    "ex\030\002 \001(\005\022\023\n\013numClusters\030\003 \001(\005\022,\n\006tracks\030"
    "\005 \003(\0132\034.motionClustering.Trajectory\"<\n\tV"
    "ideoList\022/\n\006videos\030\001 \003(\0132\037.motionCluster"
    "ing.VideoInstanceb\006proto3", 265);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "dump.proto", &protobuf_RegisterTypes);
  Trajectory::default_instance_ = new Trajectory();
  VideoInstance::default_instance_ = new VideoInstance();
  VideoList::default_instance_ = new VideoList();
  Trajectory::default_instance_->InitAsDefaultInstance();
  VideoInstance::default_instance_->InitAsDefaultInstance();
  VideoList::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_dump_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_dump_2eproto {
  StaticDescriptorInitializer_dump_2eproto() {
    protobuf_AddDesc_dump_2eproto();
  }
} static_descriptor_initializer_dump_2eproto_;

namespace {

static void MergeFromFail(int line) GOOGLE_ATTRIBUTE_COLD;
static void MergeFromFail(int line) {
  GOOGLE_CHECK(false) << __FILE__ << ":" << line;
}

}  // namespace


// ===================================================================

#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int Trajectory::kNormalizedPointsFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

Trajectory::Trajectory()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  SharedCtor();
  // @@protoc_insertion_point(constructor:motionClustering.Trajectory)
}

void Trajectory::InitAsDefaultInstance() {
  _is_default_instance_ = true;
}

Trajectory::Trajectory(const Trajectory& from)
  : ::google::protobuf::Message(),
    _internal_metadata_(NULL) {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:motionClustering.Trajectory)
}

void Trajectory::SharedCtor() {
    _is_default_instance_ = false;
  _cached_size_ = 0;
}

Trajectory::~Trajectory() {
  // @@protoc_insertion_point(destructor:motionClustering.Trajectory)
  SharedDtor();
}

void Trajectory::SharedDtor() {
  if (this != default_instance_) {
  }
}

void Trajectory::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* Trajectory::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return Trajectory_descriptor_;
}

const Trajectory& Trajectory::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_dump_2eproto();
  return *default_instance_;
}

Trajectory* Trajectory::default_instance_ = NULL;

Trajectory* Trajectory::New(::google::protobuf::Arena* arena) const {
  Trajectory* n = new Trajectory;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void Trajectory::Clear() {
  normalizedpoints_.Clear();
}

bool Trajectory::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:motionClustering.Trajectory)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // repeated float normalizedPoints = 1;
      case 1: {
        if (tag == 10) {
          DO_((::google::protobuf::internal::WireFormatLite::ReadPackedPrimitive<
                   float, ::google::protobuf::internal::WireFormatLite::TYPE_FLOAT>(
                 input, this->mutable_normalizedpoints())));
        } else if (tag == 13) {
          DO_((::google::protobuf::internal::WireFormatLite::ReadRepeatedPrimitiveNoInline<
                   float, ::google::protobuf::internal::WireFormatLite::TYPE_FLOAT>(
                 1, 10, input, this->mutable_normalizedpoints())));
        } else {
          goto handle_unusual;
        }
        if (input->ExpectAtEnd()) goto success;
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0 ||
            ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormatLite::SkipField(input, tag));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:motionClustering.Trajectory)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:motionClustering.Trajectory)
  return false;
#undef DO_
}

void Trajectory::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:motionClustering.Trajectory)
  // repeated float normalizedPoints = 1;
  if (this->normalizedpoints_size() > 0) {
    ::google::protobuf::internal::WireFormatLite::WriteTag(1, ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED, output);
    output->WriteVarint32(_normalizedpoints_cached_byte_size_);
  }
  for (int i = 0; i < this->normalizedpoints_size(); i++) {
    ::google::protobuf::internal::WireFormatLite::WriteFloatNoTag(
      this->normalizedpoints(i), output);
  }

  // @@protoc_insertion_point(serialize_end:motionClustering.Trajectory)
}

::google::protobuf::uint8* Trajectory::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:motionClustering.Trajectory)
  // repeated float normalizedPoints = 1;
  if (this->normalizedpoints_size() > 0) {
    target = ::google::protobuf::internal::WireFormatLite::WriteTagToArray(
      1,
      ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED,
      target);
    target = ::google::protobuf::io::CodedOutputStream::WriteVarint32ToArray(
      _normalizedpoints_cached_byte_size_, target);
  }
  for (int i = 0; i < this->normalizedpoints_size(); i++) {
    target = ::google::protobuf::internal::WireFormatLite::
      WriteFloatNoTagToArray(this->normalizedpoints(i), target);
  }

  // @@protoc_insertion_point(serialize_to_array_end:motionClustering.Trajectory)
  return target;
}

int Trajectory::ByteSize() const {
  int total_size = 0;

  // repeated float normalizedPoints = 1;
  {
    int data_size = 0;
    data_size = 4 * this->normalizedpoints_size();
    if (data_size > 0) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(data_size);
    }
    GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
    _normalizedpoints_cached_byte_size_ = data_size;
    GOOGLE_SAFE_CONCURRENT_WRITES_END();
    total_size += data_size;
  }

  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void Trajectory::MergeFrom(const ::google::protobuf::Message& from) {
  if (GOOGLE_PREDICT_FALSE(&from == this)) MergeFromFail(__LINE__);
  const Trajectory* source = 
      ::google::protobuf::internal::DynamicCastToGenerated<const Trajectory>(
          &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void Trajectory::MergeFrom(const Trajectory& from) {
  if (GOOGLE_PREDICT_FALSE(&from == this)) MergeFromFail(__LINE__);
  normalizedpoints_.MergeFrom(from.normalizedpoints_);
}

void Trajectory::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void Trajectory::CopyFrom(const Trajectory& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool Trajectory::IsInitialized() const {

  return true;
}

void Trajectory::Swap(Trajectory* other) {
  if (other == this) return;
  InternalSwap(other);
}
void Trajectory::InternalSwap(Trajectory* other) {
  normalizedpoints_.UnsafeArenaSwap(&other->normalizedpoints_);
  _internal_metadata_.Swap(&other->_internal_metadata_);
  std::swap(_cached_size_, other->_cached_size_);
}

::google::protobuf::Metadata Trajectory::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = Trajectory_descriptor_;
  metadata.reflection = Trajectory_reflection_;
  return metadata;
}

#if PROTOBUF_INLINE_NOT_IN_HEADERS
// Trajectory

// repeated float normalizedPoints = 1;
int Trajectory::normalizedpoints_size() const {
  return normalizedpoints_.size();
}
void Trajectory::clear_normalizedpoints() {
  normalizedpoints_.Clear();
}
 float Trajectory::normalizedpoints(int index) const {
  // @@protoc_insertion_point(field_get:motionClustering.Trajectory.normalizedPoints)
  return normalizedpoints_.Get(index);
}
 void Trajectory::set_normalizedpoints(int index, float value) {
  normalizedpoints_.Set(index, value);
  // @@protoc_insertion_point(field_set:motionClustering.Trajectory.normalizedPoints)
}
 void Trajectory::add_normalizedpoints(float value) {
  normalizedpoints_.Add(value);
  // @@protoc_insertion_point(field_add:motionClustering.Trajectory.normalizedPoints)
}
 const ::google::protobuf::RepeatedField< float >&
Trajectory::normalizedpoints() const {
  // @@protoc_insertion_point(field_list:motionClustering.Trajectory.normalizedPoints)
  return normalizedpoints_;
}
 ::google::protobuf::RepeatedField< float >*
Trajectory::mutable_normalizedpoints() {
  // @@protoc_insertion_point(field_mutable_list:motionClustering.Trajectory.normalizedPoints)
  return &normalizedpoints_;
}

#endif  // PROTOBUF_INLINE_NOT_IN_HEADERS

// ===================================================================

#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int VideoInstance::kActionLabelFieldNumber;
const int VideoInstance::kVideoIndexFieldNumber;
const int VideoInstance::kNumClustersFieldNumber;
const int VideoInstance::kTracksFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

VideoInstance::VideoInstance()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  SharedCtor();
  // @@protoc_insertion_point(constructor:motionClustering.VideoInstance)
}

void VideoInstance::InitAsDefaultInstance() {
  _is_default_instance_ = true;
}

VideoInstance::VideoInstance(const VideoInstance& from)
  : ::google::protobuf::Message(),
    _internal_metadata_(NULL) {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:motionClustering.VideoInstance)
}

void VideoInstance::SharedCtor() {
    _is_default_instance_ = false;
  ::google::protobuf::internal::GetEmptyString();
  _cached_size_ = 0;
  actionlabel_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  videoindex_ = 0;
  numclusters_ = 0;
}

VideoInstance::~VideoInstance() {
  // @@protoc_insertion_point(destructor:motionClustering.VideoInstance)
  SharedDtor();
}

void VideoInstance::SharedDtor() {
  actionlabel_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (this != default_instance_) {
  }
}

void VideoInstance::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* VideoInstance::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return VideoInstance_descriptor_;
}

const VideoInstance& VideoInstance::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_dump_2eproto();
  return *default_instance_;
}

VideoInstance* VideoInstance::default_instance_ = NULL;

VideoInstance* VideoInstance::New(::google::protobuf::Arena* arena) const {
  VideoInstance* n = new VideoInstance;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void VideoInstance::Clear() {
#define ZR_HELPER_(f) reinterpret_cast<char*>(\
  &reinterpret_cast<VideoInstance*>(16)->f)

#define ZR_(first, last) do {\
  ::memset(&first, 0,\
           ZR_HELPER_(last) - ZR_HELPER_(first) + sizeof(last));\
} while (0)

  ZR_(videoindex_, numclusters_);
  actionlabel_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());

#undef ZR_HELPER_
#undef ZR_

  tracks_.Clear();
}

bool VideoInstance::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:motionClustering.VideoInstance)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // optional string actionLabel = 1;
      case 1: {
        if (tag == 10) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_actionlabel()));
          DO_(::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
            this->actionlabel().data(), this->actionlabel().length(),
            ::google::protobuf::internal::WireFormatLite::PARSE,
            "motionClustering.VideoInstance.actionLabel"));
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(16)) goto parse_videoIndex;
        break;
      }

      // optional int32 videoIndex = 2;
      case 2: {
        if (tag == 16) {
         parse_videoIndex:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &videoindex_)));

        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(24)) goto parse_numClusters;
        break;
      }

      // optional int32 numClusters = 3;
      case 3: {
        if (tag == 24) {
         parse_numClusters:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &numclusters_)));

        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(42)) goto parse_tracks;
        break;
      }

      // repeated .motionClustering.Trajectory tracks = 5;
      case 5: {
        if (tag == 42) {
         parse_tracks:
          DO_(input->IncrementRecursionDepth());
         parse_loop_tracks:
          DO_(::google::protobuf::internal::WireFormatLite::ReadMessageNoVirtualNoRecursionDepth(
                input, add_tracks()));
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(42)) goto parse_loop_tracks;
        input->UnsafeDecrementRecursionDepth();
        if (input->ExpectAtEnd()) goto success;
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0 ||
            ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormatLite::SkipField(input, tag));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:motionClustering.VideoInstance)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:motionClustering.VideoInstance)
  return false;
#undef DO_
}

void VideoInstance::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:motionClustering.VideoInstance)
  // optional string actionLabel = 1;
  if (this->actionlabel().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
      this->actionlabel().data(), this->actionlabel().length(),
      ::google::protobuf::internal::WireFormatLite::SERIALIZE,
      "motionClustering.VideoInstance.actionLabel");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      1, this->actionlabel(), output);
  }

  // optional int32 videoIndex = 2;
  if (this->videoindex() != 0) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(2, this->videoindex(), output);
  }

  // optional int32 numClusters = 3;
  if (this->numclusters() != 0) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(3, this->numclusters(), output);
  }

  // repeated .motionClustering.Trajectory tracks = 5;
  for (unsigned int i = 0, n = this->tracks_size(); i < n; i++) {
    ::google::protobuf::internal::WireFormatLite::WriteMessageMaybeToArray(
      5, this->tracks(i), output);
  }

  // @@protoc_insertion_point(serialize_end:motionClustering.VideoInstance)
}

::google::protobuf::uint8* VideoInstance::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:motionClustering.VideoInstance)
  // optional string actionLabel = 1;
  if (this->actionlabel().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
      this->actionlabel().data(), this->actionlabel().length(),
      ::google::protobuf::internal::WireFormatLite::SERIALIZE,
      "motionClustering.VideoInstance.actionLabel");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        1, this->actionlabel(), target);
  }

  // optional int32 videoIndex = 2;
  if (this->videoindex() != 0) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(2, this->videoindex(), target);
  }

  // optional int32 numClusters = 3;
  if (this->numclusters() != 0) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(3, this->numclusters(), target);
  }

  // repeated .motionClustering.Trajectory tracks = 5;
  for (unsigned int i = 0, n = this->tracks_size(); i < n; i++) {
    target = ::google::protobuf::internal::WireFormatLite::
      WriteMessageNoVirtualToArray(
        5, this->tracks(i), target);
  }

  // @@protoc_insertion_point(serialize_to_array_end:motionClustering.VideoInstance)
  return target;
}

int VideoInstance::ByteSize() const {
  int total_size = 0;

  // optional string actionLabel = 1;
  if (this->actionlabel().size() > 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::StringSize(
        this->actionlabel());
  }

  // optional int32 videoIndex = 2;
  if (this->videoindex() != 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::Int32Size(
        this->videoindex());
  }

  // optional int32 numClusters = 3;
  if (this->numclusters() != 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::Int32Size(
        this->numclusters());
  }

  // repeated .motionClustering.Trajectory tracks = 5;
  total_size += 1 * this->tracks_size();
  for (int i = 0; i < this->tracks_size(); i++) {
    total_size +=
      ::google::protobuf::internal::WireFormatLite::MessageSizeNoVirtual(
        this->tracks(i));
  }

  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void VideoInstance::MergeFrom(const ::google::protobuf::Message& from) {
  if (GOOGLE_PREDICT_FALSE(&from == this)) MergeFromFail(__LINE__);
  const VideoInstance* source = 
      ::google::protobuf::internal::DynamicCastToGenerated<const VideoInstance>(
          &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void VideoInstance::MergeFrom(const VideoInstance& from) {
  if (GOOGLE_PREDICT_FALSE(&from == this)) MergeFromFail(__LINE__);
  tracks_.MergeFrom(from.tracks_);
  if (from.actionlabel().size() > 0) {

    actionlabel_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.actionlabel_);
  }
  if (from.videoindex() != 0) {
    set_videoindex(from.videoindex());
  }
  if (from.numclusters() != 0) {
    set_numclusters(from.numclusters());
  }
}

void VideoInstance::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void VideoInstance::CopyFrom(const VideoInstance& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool VideoInstance::IsInitialized() const {

  return true;
}

void VideoInstance::Swap(VideoInstance* other) {
  if (other == this) return;
  InternalSwap(other);
}
void VideoInstance::InternalSwap(VideoInstance* other) {
  actionlabel_.Swap(&other->actionlabel_);
  std::swap(videoindex_, other->videoindex_);
  std::swap(numclusters_, other->numclusters_);
  tracks_.UnsafeArenaSwap(&other->tracks_);
  _internal_metadata_.Swap(&other->_internal_metadata_);
  std::swap(_cached_size_, other->_cached_size_);
}

::google::protobuf::Metadata VideoInstance::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = VideoInstance_descriptor_;
  metadata.reflection = VideoInstance_reflection_;
  return metadata;
}

#if PROTOBUF_INLINE_NOT_IN_HEADERS
// VideoInstance

// optional string actionLabel = 1;
void VideoInstance::clear_actionlabel() {
  actionlabel_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 const ::std::string& VideoInstance::actionlabel() const {
  // @@protoc_insertion_point(field_get:motionClustering.VideoInstance.actionLabel)
  return actionlabel_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 void VideoInstance::set_actionlabel(const ::std::string& value) {
  
  actionlabel_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:motionClustering.VideoInstance.actionLabel)
}
 void VideoInstance::set_actionlabel(const char* value) {
  
  actionlabel_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:motionClustering.VideoInstance.actionLabel)
}
 void VideoInstance::set_actionlabel(const char* value, size_t size) {
  
  actionlabel_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:motionClustering.VideoInstance.actionLabel)
}
 ::std::string* VideoInstance::mutable_actionlabel() {
  
  // @@protoc_insertion_point(field_mutable:motionClustering.VideoInstance.actionLabel)
  return actionlabel_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 ::std::string* VideoInstance::release_actionlabel() {
  
  return actionlabel_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 void VideoInstance::set_allocated_actionlabel(::std::string* actionlabel) {
  if (actionlabel != NULL) {
    
  } else {
    
  }
  actionlabel_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), actionlabel);
  // @@protoc_insertion_point(field_set_allocated:motionClustering.VideoInstance.actionLabel)
}

// optional int32 videoIndex = 2;
void VideoInstance::clear_videoindex() {
  videoindex_ = 0;
}
 ::google::protobuf::int32 VideoInstance::videoindex() const {
  // @@protoc_insertion_point(field_get:motionClustering.VideoInstance.videoIndex)
  return videoindex_;
}
 void VideoInstance::set_videoindex(::google::protobuf::int32 value) {
  
  videoindex_ = value;
  // @@protoc_insertion_point(field_set:motionClustering.VideoInstance.videoIndex)
}

// optional int32 numClusters = 3;
void VideoInstance::clear_numclusters() {
  numclusters_ = 0;
}
 ::google::protobuf::int32 VideoInstance::numclusters() const {
  // @@protoc_insertion_point(field_get:motionClustering.VideoInstance.numClusters)
  return numclusters_;
}
 void VideoInstance::set_numclusters(::google::protobuf::int32 value) {
  
  numclusters_ = value;
  // @@protoc_insertion_point(field_set:motionClustering.VideoInstance.numClusters)
}

// repeated .motionClustering.Trajectory tracks = 5;
int VideoInstance::tracks_size() const {
  return tracks_.size();
}
void VideoInstance::clear_tracks() {
  tracks_.Clear();
}
const ::motionClustering::Trajectory& VideoInstance::tracks(int index) const {
  // @@protoc_insertion_point(field_get:motionClustering.VideoInstance.tracks)
  return tracks_.Get(index);
}
::motionClustering::Trajectory* VideoInstance::mutable_tracks(int index) {
  // @@protoc_insertion_point(field_mutable:motionClustering.VideoInstance.tracks)
  return tracks_.Mutable(index);
}
::motionClustering::Trajectory* VideoInstance::add_tracks() {
  // @@protoc_insertion_point(field_add:motionClustering.VideoInstance.tracks)
  return tracks_.Add();
}
::google::protobuf::RepeatedPtrField< ::motionClustering::Trajectory >*
VideoInstance::mutable_tracks() {
  // @@protoc_insertion_point(field_mutable_list:motionClustering.VideoInstance.tracks)
  return &tracks_;
}
const ::google::protobuf::RepeatedPtrField< ::motionClustering::Trajectory >&
VideoInstance::tracks() const {
  // @@protoc_insertion_point(field_list:motionClustering.VideoInstance.tracks)
  return tracks_;
}

#endif  // PROTOBUF_INLINE_NOT_IN_HEADERS

// ===================================================================

#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int VideoList::kVideosFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

VideoList::VideoList()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  SharedCtor();
  // @@protoc_insertion_point(constructor:motionClustering.VideoList)
}

void VideoList::InitAsDefaultInstance() {
  _is_default_instance_ = true;
}

VideoList::VideoList(const VideoList& from)
  : ::google::protobuf::Message(),
    _internal_metadata_(NULL) {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:motionClustering.VideoList)
}

void VideoList::SharedCtor() {
    _is_default_instance_ = false;
  _cached_size_ = 0;
}

VideoList::~VideoList() {
  // @@protoc_insertion_point(destructor:motionClustering.VideoList)
  SharedDtor();
}

void VideoList::SharedDtor() {
  if (this != default_instance_) {
  }
}

void VideoList::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* VideoList::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return VideoList_descriptor_;
}

const VideoList& VideoList::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_dump_2eproto();
  return *default_instance_;
}

VideoList* VideoList::default_instance_ = NULL;

VideoList* VideoList::New(::google::protobuf::Arena* arena) const {
  VideoList* n = new VideoList;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void VideoList::Clear() {
  videos_.Clear();
}

bool VideoList::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:motionClustering.VideoList)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // repeated .motionClustering.VideoInstance videos = 1;
      case 1: {
        if (tag == 10) {
          DO_(input->IncrementRecursionDepth());
         parse_loop_videos:
          DO_(::google::protobuf::internal::WireFormatLite::ReadMessageNoVirtualNoRecursionDepth(
                input, add_videos()));
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(10)) goto parse_loop_videos;
        input->UnsafeDecrementRecursionDepth();
        if (input->ExpectAtEnd()) goto success;
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0 ||
            ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormatLite::SkipField(input, tag));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:motionClustering.VideoList)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:motionClustering.VideoList)
  return false;
#undef DO_
}

void VideoList::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:motionClustering.VideoList)
  // repeated .motionClustering.VideoInstance videos = 1;
  for (unsigned int i = 0, n = this->videos_size(); i < n; i++) {
    ::google::protobuf::internal::WireFormatLite::WriteMessageMaybeToArray(
      1, this->videos(i), output);
  }

  // @@protoc_insertion_point(serialize_end:motionClustering.VideoList)
}

::google::protobuf::uint8* VideoList::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:motionClustering.VideoList)
  // repeated .motionClustering.VideoInstance videos = 1;
  for (unsigned int i = 0, n = this->videos_size(); i < n; i++) {
    target = ::google::protobuf::internal::WireFormatLite::
      WriteMessageNoVirtualToArray(
        1, this->videos(i), target);
  }

  // @@protoc_insertion_point(serialize_to_array_end:motionClustering.VideoList)
  return target;
}

int VideoList::ByteSize() const {
  int total_size = 0;

  // repeated .motionClustering.VideoInstance videos = 1;
  total_size += 1 * this->videos_size();
  for (int i = 0; i < this->videos_size(); i++) {
    total_size +=
      ::google::protobuf::internal::WireFormatLite::MessageSizeNoVirtual(
        this->videos(i));
  }

  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void VideoList::MergeFrom(const ::google::protobuf::Message& from) {
  if (GOOGLE_PREDICT_FALSE(&from == this)) MergeFromFail(__LINE__);
  const VideoList* source = 
      ::google::protobuf::internal::DynamicCastToGenerated<const VideoList>(
          &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void VideoList::MergeFrom(const VideoList& from) {
  if (GOOGLE_PREDICT_FALSE(&from == this)) MergeFromFail(__LINE__);
  videos_.MergeFrom(from.videos_);
}

void VideoList::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void VideoList::CopyFrom(const VideoList& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool VideoList::IsInitialized() const {

  return true;
}

void VideoList::Swap(VideoList* other) {
  if (other == this) return;
  InternalSwap(other);
}
void VideoList::InternalSwap(VideoList* other) {
  videos_.UnsafeArenaSwap(&other->videos_);
  _internal_metadata_.Swap(&other->_internal_metadata_);
  std::swap(_cached_size_, other->_cached_size_);
}

::google::protobuf::Metadata VideoList::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = VideoList_descriptor_;
  metadata.reflection = VideoList_reflection_;
  return metadata;
}

#if PROTOBUF_INLINE_NOT_IN_HEADERS
// VideoList

// repeated .motionClustering.VideoInstance videos = 1;
int VideoList::videos_size() const {
  return videos_.size();
}
void VideoList::clear_videos() {
  videos_.Clear();
}
const ::motionClustering::VideoInstance& VideoList::videos(int index) const {
  // @@protoc_insertion_point(field_get:motionClustering.VideoList.videos)
  return videos_.Get(index);
}
::motionClustering::VideoInstance* VideoList::mutable_videos(int index) {
  // @@protoc_insertion_point(field_mutable:motionClustering.VideoList.videos)
  return videos_.Mutable(index);
}
::motionClustering::VideoInstance* VideoList::add_videos() {
  // @@protoc_insertion_point(field_add:motionClustering.VideoList.videos)
  return videos_.Add();
}
::google::protobuf::RepeatedPtrField< ::motionClustering::VideoInstance >*
VideoList::mutable_videos() {
  // @@protoc_insertion_point(field_mutable_list:motionClustering.VideoList.videos)
  return &videos_;
}
const ::google::protobuf::RepeatedPtrField< ::motionClustering::VideoInstance >&
VideoList::videos() const {
  // @@protoc_insertion_point(field_list:motionClustering.VideoList.videos)
  return videos_;
}

#endif  // PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)

}  // namespace motionClustering

// @@protoc_insertion_point(global_scope)
