#include "isb/hub/hub.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
namespace fs=std::filesystem; namespace isb::hub {
namespace { std::string q(const std::string&s){std::string r="\"";for(char c:s){if(c=='"'||c=='\\')r+='\\';r+=c;}return r+"\"";} std::string now(bool synthetic){return synthetic?"1970-01-01T00:00:00Z":"unknown";} cal::GpuCapabilities mock_caps(){cal::GpuCapabilities c;c.identity={"NVIDIA","Volta",{{7,0}},cal::GpuVariant::V100_SXM2,"Tesla V100 SXM2 16 GB","Tesla V100-SXM2-16GB"};c.compute.cuda_state=cal::CapabilityState::Unknown;c.compute.compute_capability={{7,0}};c.compute.tensor_cores={cal::CapabilityState::Available,1};c.compute.tensor_precisions={cal::CapabilityState::Available,{cal::TensorPrecision::FP16}};c.graphics.vulkan_state=cal::CapabilityState::Unknown;c.hardware.tensor_cores={cal::CapabilityState::Available,1};c.hardware.hbm2.state=cal::CapabilityState::Available;c.hardware.ecc.state=cal::CapabilityState::Available;c.hardware.nvlink.state=cal::CapabilityState::Unknown;c.hardware.rt_cores.state=cal::CapabilityState::Unavailable;c.hardware.optical_flow_accelerator.state=cal::CapabilityState::Unavailable;c.hardware.mig.state=cal::CapabilityState::Unavailable;c.hardware.display_outputs.state=cal::CapabilityState::Unavailable;return c;} }
const char* to_string(ProviderMode x){return x==ProviderMode::Mock?"mock":x==ProviderMode::Real?"real":"unavailable";} const char* to_string(ControlState x){switch(x){case ControlState::Supported:return "supported";case ControlState::Unsupported:return "unsupported";case ControlState::PermissionDenied:return "permission_denied";case ControlState::Error:return "error";default:return "unknown";}}
const char* to_string(OperationState x){switch(x){case OperationState::Planned:return "planned";case OperationState::Applied:return "applied";case OperationState::Verified:return "verified";case OperationState::Failed:return "failed";case OperationState::RolledBack:return "rolled_back";}return "failed";}
Environment MockProvider::environment() const { return {"mock-linux","mock-driver-0",ProviderMode::Mock,{"mock-provider","deterministic V100 fixture",true}}; }
cal::GpuCapabilities MockProvider::capabilities() const { return mock_caps(); }
TelemetrySnapshot MockProvider::telemetry() const {
    TelemetrySnapshot t;
    t.timestamp = now(true); t.synthetic = true;
    t.provenance = {"mock-provider","deterministic telemetry fixture",true};
    t.temperature_c=42; t.gpu_utilization_percent=37; t.memory_utilization_percent=28;
    t.power_w=145; t.power_limit_w=300; t.gpu_clock_mhz=1230; t.memory_clock_mhz=877;
    t.vram_used_mib=2048; t.vram_total_mib=16160; t.performance_state="P0";
    t.ecc="enabled"; t.pcie="unknown"; t.nvlink="unknown"; t.process_count=0;
    t.driver="mock-driver-0"; return t;
}
std::vector<Control> MockProvider::controls() const {
    return {{"persistence_mode",ControlState::Unknown,"mock cannot prove management API support"},
            {"power_limit",ControlState::Unknown,"mock cannot safely expose mutation"},
            {"application_clocks",ControlState::Unknown,"mock cannot safely expose mutation"},
            {"compute_mode",ControlState::Unknown,"mock cannot safely expose mutation"}};
}
Environment UnavailableProvider::environment()const{return {"unknown","unknown",ProviderMode::Unavailable,{"unavailable-provider","no optional hardware provider compiled or detected",false}};} cal::GpuCapabilities UnavailableProvider::capabilities()const{return {};} TelemetrySnapshot UnavailableProvider::telemetry() const {
    TelemetrySnapshot t; t.timestamp=now(false);
    t.provenance={"unavailable-provider","NVML/CUDA/Vulkan unavailable",false};
    t.driver="unknown"; t.performance_state="unknown"; t.ecc="unknown";
    t.pcie="unknown"; t.nvlink="unknown"; return t;
} std::vector<Control> UnavailableProvider::controls()const{return {{"persistence_mode",ControlState::Unknown,"NVML unavailable"},{"power_limit",ControlState::Unknown,"NVML unavailable"},{"application_clocks",ControlState::Unknown,"NVML unavailable"},{"compute_mode",ControlState::Unknown,"NVML unavailable"}};}
Hub::Hub(const Provider&p):provider_(p){} Environment Hub::environment()const{return provider_.environment();} CapabilitySnapshot Hub::capability_snapshot()const{return {provider_.capabilities(),provider_.environment().provenance};} cal::GpuCapabilities Hub::capabilities()const{return capability_snapshot().capabilities;} TelemetrySnapshot Hub::telemetry()const{return provider_.telemetry();} std::vector<Control> Hub::controls()const{return provider_.controls();} std::vector<std::string> Hub::profiles()const{return {"Balanced","Gaming","Compute","AI/Tensor","Maximum Performance","Low Power","Custom"};}
OperationPlan Hub::profile_plan(const std::string& p)const{OperationPlan plan{"profile-"+p,true};for(const auto&c:controls()){if(c.state==ControlState::Supported)plan.operations.push_back({c.name,"Apply "+p+" policy",c.state,true,true,"read back previous value"});else if(c.state==ControlState::Unknown)plan.unknown.push_back(c.name+": "+c.reason);else plan.unsupported.push_back(c.name+": "+c.reason);}plan.risks={"Profiles are policy templates, not guaranteed optimal settings."};plan.verification_steps={"Read back every applied setting.","Do not report success before verification."};return plan;}
OperationPlan Hub::optimize_plan()const{auto p=profile_plan("Balanced");p.id="optimize-v100";p.risks.push_back("Dry run performs zero mutations.");return p;} std::vector<ErrorEvent> Hub::diagnose()const{auto e=environment();if(e.mode==ProviderMode::Unavailable)return {{"ISB_PROVIDER_UNAVAILABLE","Hardware evidence is unavailable; no fix is automatically approved.",e.provenance}};return {{"ISB_SYNTHETIC_EVIDENCE","Mock evidence is not proof of real V100 behavior.",e.provenance}};} VerificationResult Hub::verify()const{auto e=environment();return {false,"ISB_VERIFY_NO_APPLY","No operation has been applied in this stateless backend session.",e.provenance};} BenchmarkResult Hub::benchmark()const{auto e=environment();return {"v100-fixture","1",e.mode==ProviderMode::Mock,true,"fixture only; no performance claim",e.provenance};} ApplyResult Hub::apply(const OperationPlan&p,bool approved)const{if(!approved)return {OperationState::Planned,false,{false,"ISB_APPROVAL_REQUIRED","Explicit approval is required before mutation.",environment().provenance}};if(p.operations.empty())return {OperationState::Failed,false,{false,"ISB_NO_EXECUTABLE_OPERATION","No supported, provider-backed operation is available.",environment().provenance}};return {OperationState::Failed,false,{false,"ISB_MUTATION_PROVIDER_UNIMPLEMENTED","Read-only provider path cannot mutate hardware.",environment().provenance}};}
std::string json(const Environment&e){return std::string("{\"mode\":")+q(to_string(e.mode))+",\"os\":"+q(e.os)+",\"driver\":"+q(e.driver_version)+",\"synthetic\":"+(e.provenance.synthetic?"true":"false")+",\"provenance\":"+q(e.provenance.provider)+"}";} template <typename T> void json_optional(std::ostringstream& o, const std::optional<T>& v){ if(v) o<<*v; else o<<"null"; }
std::string json(const TelemetrySnapshot&t){
    std::ostringstream o;
    o<<"{\"timestamp\":"<<q(t.timestamp)<<",\"synthetic\":"<<(t.synthetic?"true":"false");
    o<<",\"temperature_c\":"; json_optional(o,t.temperature_c);
    o<<",\"gpu_utilization_percent\":"; json_optional(o,t.gpu_utilization_percent);
    o<<",\"memory_utilization_percent\":"; json_optional(o,t.memory_utilization_percent);
    o<<",\"power_usage_w\":"; json_optional(o,t.power_w);
    o<<",\"power_limit_w\":"; json_optional(o,t.power_limit_w);
    o<<",\"gpu_clock_mhz\":"; json_optional(o,t.gpu_clock_mhz);
    o<<",\"memory_clock_mhz\":"; json_optional(o,t.memory_clock_mhz);
    o<<",\"vram_used_mib\":"; json_optional(o,t.vram_used_mib);
    o<<",\"vram_total_mib\":"; json_optional(o,t.vram_total_mib);
    o<<",\"performance_state\":"<<q(t.performance_state)
      <<",\"ecc\":"<<q(t.ecc)<<",\"pcie\":"<<q(t.pcie)<<",\"nvlink\":"<<q(t.nvlink);
    o<<",\"process_count\":"; json_optional(o,t.process_count);
    o<<",\"driver\":"<<q(t.driver)<<"}";
    return o.str();
} std::string json(const OperationPlan&p){std::ostringstream o;o<<"{\"id\":"<<q(p.id)<<",\"dry_run\":"<<(p.dry_run?"true":"false")<<",\"operations\":[] ,\"unknown\":[";for(size_t i=0;i<p.unknown.size();++i){if(i)o<<',';o<<q(p.unknown[i]);}o<<"]}";return o.str();} std::string json(const BenchmarkResult&b){return std::string("{\"name\":")+q(b.name)+",\"version\":"+q(b.version)+",\"synthetic\":"+(b.synthetic?"true":"false")+",\"correctness_verified\":"+(b.correctness_verified?"true":"false")+",\"result\":"+q(b.result)+"}";} std::string json(const VerificationResult&v){return std::string("{\"verified\":")+(v.verified?"true":"false")+",\"code\":"+q(v.code)+",\"message\":"+q(v.message)+"}";}
std::string Hub::status_json()const{const auto e=environment();const auto c=capability_snapshot();const auto t=telemetry();std::ostringstream o;o<<"{\"schema_version\":\"1\",\"mode\":"<<q(to_string(e.mode))<<",\"environment\":"<<json(e)<<",\"capabilities\":"<<cal::to_json(c.capabilities)<<",\"telemetry\":"<<json(t)<<",\"provenance\":{\"provider\":"<<q(c.provenance.provider)<<",\"synthetic\":"<<(c.provenance.synthetic?"true":"false")<<"}}";return o.str();}
ReportBundle Hub::report(const std::string&d)const{fs::create_directories(fs::path(d)/"logs");std::ofstream(d+"/gpu.json")<<cal::to_json(capabilities());std::ofstream(d+"/capabilities.json")<<cal::to_json(capabilities());std::ofstream(d+"/driver.json")<<json(environment());std::ofstream(d+"/performance.json")<<json(telemetry());std::ofstream(d+"/games.json")<<"[]\n";std::ofstream(d+"/optiscaler.json")<<"{\"state\":\"unknown\",\"reason\":\"external component not detected\"}\n";std::ofstream(d+"/errors.json")<<"[]\n";const auto synthetic=environment().provenance.synthetic;std::string m=std::string("{\"schema_version\":\"1\",\"isb_version\":\"0.1\",\"synthetic\":")+(synthetic?"true":"false")+",\"environment_hash\":\"deterministic-mock-v1\"}\n";std::ofstream(d+"/manifest.json")<<m;return {d,m};}
}
