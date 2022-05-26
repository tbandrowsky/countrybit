#pragma once

namespace proposal
{
	using namespace corona::win32;


	class wsProposalController : public corona_controller {


	public:

		wsProposalController(viewStyle* _vs);
		virtual ~wsProposalController();

		virtual void updateState(corona::database::actor_state& state, const rectDto& newSize);
		virtual void loadController();
		virtual void drawFrame();
		virtual void onInit();

	};

}
