#include "save_block_data_task.h"
#include "../util/godot/funcs.h"
#include "../util/macros.h"
#include "../util/profiling.h"
#include "generate_block_task.h"
#include "voxel_server.h"

namespace zylann::voxel {

namespace {
std::atomic_int g_debug_save_block_tasks_count;
}

SaveBlockDataTask::SaveBlockDataTask(uint32_t p_volume_id, Vector3i p_block_pos, uint8_t p_lod, uint8_t p_block_size,
		std::shared_ptr<VoxelBufferInternal> p_voxels, std::shared_ptr<StreamingDependency> p_stream_dependency) :
		_voxels(p_voxels),
		_position(p_block_pos),
		_volume_id(p_volume_id),
		_lod(p_lod),
		_block_size(p_block_size),
		_save_instances(false),
		_save_voxels(true),
		_stream_dependency(p_stream_dependency) {
	//
	++g_debug_save_block_tasks_count;
}

SaveBlockDataTask::SaveBlockDataTask(uint32_t p_volume_id, Vector3i p_block_pos, uint8_t p_lod, uint8_t p_block_size,
		std::unique_ptr<InstanceBlockData> p_instances, std::shared_ptr<StreamingDependency> p_stream_dependency) :
		_instances(std::move(p_instances)),
		_position(p_block_pos),
		_volume_id(p_volume_id),
		_lod(p_lod),
		_block_size(p_block_size),
		_save_instances(true),
		_save_voxels(false),
		_stream_dependency(p_stream_dependency) {
	//
	++g_debug_save_block_tasks_count;
}

SaveBlockDataTask::~SaveBlockDataTask() {
	--g_debug_save_block_tasks_count;
}

int SaveBlockDataTask::debug_get_running_count() {
	return g_debug_save_block_tasks_count;
}

void SaveBlockDataTask::run(zylann::ThreadedTaskContext ctx) {
	VOXEL_PROFILE_SCOPE();

	CRASH_COND(_stream_dependency == nullptr);
	Ref<VoxelStream> stream = _stream_dependency->stream;
	CRASH_COND(stream.is_null());

	const Vector3i origin_in_voxels = (_position << _lod) * _block_size;

	if (_save_voxels) {
		ERR_FAIL_COND(_voxels == nullptr);
		VoxelBufferInternal voxels_copy;
		{
			RWLockRead lock(_voxels->get_lock());
			// TODO Optimization: is that copy necessary? It's possible it was already done while issuing the
			// request
			_voxels->duplicate_to(voxels_copy, true);
		}
		_voxels = nullptr;
		stream->save_voxel_block(voxels_copy, origin_in_voxels, _lod);
	}

	if (_save_instances && stream->supports_instance_blocks()) {
		// If the provided data is null, it means this instance block was never modified.
		// Since we are in a save request, the saved data will revert to unmodified.
		// On the other hand, if we want to represent the fact that "everything was deleted here",
		// this should not be null.

		PRINT_VERBOSE(String("Saving instance block {0} lod {1} with data {2}")
							  .format(varray(_position, _lod, ptr2s(_instances.get()))));

		VoxelStreamInstanceDataRequest instance_data_request;
		instance_data_request.lod = _lod;
		instance_data_request.position = _position;
		instance_data_request.data = std::move(_instances);
		stream->save_instance_blocks(Span<VoxelStreamInstanceDataRequest>(&instance_data_request, 1));
	}

	_has_run = true;
}

int SaveBlockDataTask::get_priority() {
	return 0;
}

bool SaveBlockDataTask::is_cancelled() {
	return false;
}

void SaveBlockDataTask::apply_result() {
	if (VoxelServer::get_singleton()->is_volume_valid(_volume_id)) {
		if (_stream_dependency->valid) {
			// TODO Perhaps separate save and load callbacks?
			VoxelServer::BlockDataOutput o;
			o.position = _position;
			o.lod = _lod;
			o.dropped = !_has_run;
			o.max_lod_hint = false; // Unused
			o.initial_load = false; // Unused
			o.type = VoxelServer::BlockDataOutput::TYPE_SAVED;

			VoxelServer::VolumeCallbacks callbacks = VoxelServer::get_singleton()->get_volume_callbacks(_volume_id);
			CRASH_COND(callbacks.data_output_callback == nullptr);
			callbacks.data_output_callback(callbacks.data, o);
		}

	} else {
		// This can happen if the user removes the volume while requests are still about to return
		PRINT_VERBOSE("Stream data request response came back but volume wasn't found");
	}
}

} // namespace zylann::voxel