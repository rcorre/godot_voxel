#ifndef VOXEL_STREAM_SQLITE_H
#define VOXEL_STREAM_SQLITE_H

#include "../../util/thread/mutex.h"
#include "../voxel_block_serializer.h"
#include "../voxel_stream.h"
#include "../voxel_stream_cache.h"
#include <vector>

namespace zylann::voxel {

class VoxelStreamSQLiteInternal;

// Saves voxel data into a single SQLite database file.
class VoxelStreamSQLite : public VoxelStream {
	GDCLASS(VoxelStreamSQLite, VoxelStream)
public:
	static const unsigned int CACHE_SIZE = 64;

	VoxelStreamSQLite();
	~VoxelStreamSQLite();

	void set_database_path(String path);
	String get_database_path() const;

	void load_voxel_block(VoxelStream::VoxelQueryData &q) override;
	void save_voxel_block(VoxelStream::VoxelQueryData &q) override;

	void load_voxel_blocks(Span<VoxelStream::VoxelQueryData> p_blocks) override;
	void save_voxel_blocks(Span<VoxelStream::VoxelQueryData> p_blocks) override;

	bool supports_instance_blocks() const override;
	void load_instance_blocks(Span<VoxelStream::InstancesQueryData> out_blocks) override;
	void save_instance_blocks(Span<VoxelStream::InstancesQueryData> p_blocks) override;

	bool supports_loading_all_blocks() const override {
		return true;
	}
	void load_all_blocks(FullLoadingResult &result) override;

	int get_used_channels_mask() const override;

	void flush_cache();

private:
	// An SQlite3 database is safe to use with multiple threads in serialized mode,
	// but after having a look at the implementation while stepping with a debugger, here are what actually happens:
	//
	// 1) Prepared statements might be safe to use in multiple threads, but the end result isn't safe.
	//    Thread A could bind a value, then thread B could bind another value replacing the first before thread A
	//    executes the statement. So in the end, each thread should get its own set of statements.
	//
	// 2) Executing a statement locks the entire database with a mutex.
	//    So indeed access is serialized, in the sense that CPU work will execute in series, not in parallel.
	//    in other words, you loose the speed of multi-threading.
	//
	// Because of this, in our use case, it might be simpler to just leave SQLite in thread-safe mode,
	// and synchronize ourselves.

	VoxelStreamSQLiteInternal *get_connection();
	void recycle_connection(VoxelStreamSQLiteInternal *con);
	void flush_cache(VoxelStreamSQLiteInternal *con);

	static void _bind_methods();

	String _connection_path;
	std::vector<VoxelStreamSQLiteInternal *> _connection_pool;
	Mutex _connection_mutex;
	VoxelStreamCache _cache;

	// TODO I should consider specialized memory allocators
	static thread_local std::vector<uint8_t> _temp_block_data;
	static thread_local std::vector<uint8_t> _temp_compressed_block_data;
};

} // namespace zylann::voxel

#endif // VOXEL_STREAM_SQLITE_H
