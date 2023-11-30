#pragma once

#include <d3d12.h>

#include <mutex>
#include <map>
#include <unordered_map>
#include <vector>

class CommandList;
class Resource;

class ResourceStateTracker
{
public:
	ResourceStateTracker();
	virtual ~ResourceStateTracker();
	///
	///	Push a resource barrier to the resource state tracker
	/// 
	/// @param barrier, The resource barrier to push to the resource state tracker
	/// 
	void ResourceBarrier(const D3D12_RESOURCE_BARRIER& barrier);

	///
	///	Push a transition resource barrier to the resource state tracker
	/// 
	/// @param resource (the resource to transition)
	/// @param stateAfter (the state to transition the resource to)
	/// @param subResource (the sub resource to transition, by default
	/// this is D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, meaning all subresources)
	/// 
	void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
	void TransitionResource(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

	///
	///	Push a UAV resource barrier for the given resource
	/// 
	/// @param resource (the resource to add a UAV barrier for. Can be NULL,
	/// Indicating that any UAV access could require the barrier
	/// 
	void UAVBarrier(const Resource* resource = nullptr);

	///
	///	Push an aliasing barrier for the given resource
	/// 
	/// @param beforeResource (resource currently occupying the space in the heap)
	/// @param afterResource (resource that will be occupying the space in the heap)
	/// 
	/// Either param can be NULL, indicating that any placed or reserved resource could cause aliasing
	/// 
	void AliasBarrier(const Resource* resourceBefore = nullptr, const Resource* resourceAfter = nullptr);

	///
	///	Flush any pending resource barriers to the command list
	/// @return The number of resource barriers that were flushed to the command list
	/// 
	uint32_t FlushPendingResourceBarriers(CommandList& commandList);

	//Flush any (non-pending) resource barriers that have been pushed to the resource state tracker
	void FlushResourceBarriers(CommandList& commandList); //Flushed to the command list

	//Commit final resource states to the global resource state map
	//This must be called when the command list is closed
	void CommitFinalResourceStates();

	//Reset state tracking, must be done when the command list is reset
	void Reset();

	///
	///	The global state must be locked before flushing pending resource barriers
	/// and committing the final resource state to the global resource state
	/// this ensures consistency of the global resource state between command list executions
	/// 
	static void Lock();

	///
	///	Unlocks the global resource state after the final states have been committed
	/// to the global resource state array
	/// 
	static void Unlock();

	///
	///	Add a resource with a given state to the global resource state array (map)
	/// This should be done when the resource is created for the first time
	/// 
	static void AddGlobalResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES state);

	///
	///	Remove a resource from the global resource state array (map)
	/// This should only be done when the resource is destroyed
	/// 
	static void RemoveGlobalResourceState(ID3D12Resource* resource);

private:
	//An array (vector) of resource barriers
	using ResourceBarriers = std::vector<D3D12_RESOURCE_BARRIER>;

	//Pending resource transitions are committed before a command list
	//is executed on the command queue. This guarantees that resources will
	//be in the expected state at the beginning of a command list
	ResourceBarriers m_PendingResourceBarriers;

	//Resource barriers that need to be committed to the command list
	ResourceBarriers m_ResourceBarriers;

	//Tracks the state of a particular resource and all of its subresources
	struct ResourceState
	{
		//Initialize all of the subresources within a resource to the given state
		explicit ResourceState(D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON)
			: State(state)
		{}

		//Set a subresource to a particular state
		void SetSubresourceState(UINT subresource, D3D12_RESOURCE_STATES state)
		{
			if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
			{
				State = state;
				SubresourceState.clear();
			}
			else
			{
				SubresourceState[subresource] = state;
			}
		}

		//Get the state of a (sub)resource within the resource.
		//If the specified subresource is not found in the SubresourceState array
		//Then the state of the resource (ALL_SUBRESOURCES) is returned
		D3D12_RESOURCE_STATES GetSubresourceState(UINT subresource) const
		{
			D3D12_RESOURCE_STATES state = State;
			const auto iter = SubresourceState.find(subresource);
			if (iter != SubresourceState.end())
			{
				state = iter->second;
			}
			return state;
		}

		//If the SubresourceState array (map) is empty, then
		//The State variable defines the state of all the subresources
		D3D12_RESOURCE_STATES State;
		std::map<UINT, D3D12_RESOURCE_STATES> SubresourceState;
	};

	using ResourceStateMap = std::unordered_map<ID3D12Resource*, ResourceState>;

	//The final (last known state) of the resources within a command list
	//The final resource state is committed to the global resource state when the
	//command list is closed but before it is executed on the command queue
	ResourceStateMap m_FinalResourceState;

	//The global resource state array (map) stores the state of a resource btwn command list execution
	static ResourceStateMap ms_GlobalResourceState;

	//The mutex protects shared access to the GlobalResourceState map
	static std::mutex ms_GlobalMutex;
	static bool ms_IsLocked;
};