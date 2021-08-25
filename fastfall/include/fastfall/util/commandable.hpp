#pragma once

#include <any>

template<class Enum, Enum T>
struct command {
	using is_command = std::false_type;
	using payload_type = void;
	using return_type = void;
};

#define COMMANDABLE_DEF_CMD( CommandType, CommandName, PayloadType, ReturnType )	\
template<>																	\
struct command<CommandType, CommandType::CommandName> {						\
	using is_command = std::true_type;										\
	using payload_type = PayloadType;										\
	using return_type = ReturnType;											\
};																			\

template<class Enum, Enum T> inline constexpr bool is_command_v = command<Enum, T>::is_command::value;
template<class Enum, Enum T> using cmd_payload_t = command<Enum, T>::payload_type;
template<class Enum, Enum T> using cmd_return_t = command<Enum, T>::return_type;

enum class Response {
	Accepted,
	Rejected,
	Unhandled
};

template<class Enum>
requires std::is_enum_v<Enum>
class Commandable {
protected:




	struct CmdResponse {
	private:
		CmdResponse(Response t_resp)
			: response(t_resp)
		{
		}

		template<class T>
		CmdResponse(Response t_resp, T t_payload)
			: response(t_resp)
			, payload(t_payload)
		{
		}
	public:
		Response response;
		std::any payload;

		template<class Enum>
		requires std::is_enum_v<Enum>
			friend class Commandable;
	};

	template<Enum C>
	inline static cmd_payload_t<Enum, C> cmd_payload(const std::any& t_payload) {
		return std::any_cast<cmd_payload_t<Enum, C>>(t_payload);
	}

	template<Enum C>
	requires std::is_void_v<cmd_return_t<Enum, C>>
		inline static CmdResponse respond(bool accepted) {
		return { accepted ? Response::Accepted : Response::Rejected };
	}

	template<Enum C>
	inline static CmdResponse respond(bool accepted, cmd_return_t<Enum, C> cmd_return) {
		return { accepted ? Response::Accepted : Response::Rejected, cmd_return };
	}

	template<Enum C>
	inline static CmdResponse respond(cmd_return_t<Enum, C> cmd_return) {
		return { Response::Accepted, cmd_return };
	}

	virtual CmdResponse do_command(Enum cmd, const std::any& payload) {
		return { Response::Unhandled };
	}

public:

	template<
		Enum C, class CmdReturnType = cmd_return_t<Enum, C>,
		class Return = std::conditional_t<std::is_void_v<CmdReturnType>, Response, std::pair<Response, CmdReturnType>>
	>
		requires is_command_v<Enum, C>&& std::is_void_v<cmd_payload_t<Enum, C>>
	constexpr inline Return command() {
		auto resp = do_command(C, std::any{});
		if constexpr (std::is_void_v<CmdReturnType>) {
			return resp.response;
		}
		else {
			return { resp.response, std::any_cast<CmdReturnType>(resp.payload) };
		}
	}

	template<
		Enum C, class CmdReturnType = cmd_return_t<Enum, C>,
		class Return = std::conditional_t<std::is_void_v<CmdReturnType>, Response, std::pair<Response, CmdReturnType>>
	>
		requires is_command_v<Enum, C>
	constexpr inline Return command(const cmd_payload_t<Enum, C>& payload) {
		auto resp = do_command(C, payload);
		if constexpr (std::is_void_v<CmdReturnType>) {
			assert(!resp.payload.has_value());
			return resp.response;
		}
		else {
			assert(resp.payload.has_value());
			return { resp.response, std::any_cast<CmdReturnType>(resp.payload) };
		}
	}
};