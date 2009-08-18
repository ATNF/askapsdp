module askap {
    module cp {
        module frontend {
            dictionary<string, string> WorkflowDesc;

            interface IFrontend {
                void startWorkflow(WorkflowDesc wfDesc);
                void stopWorkflow();
                void shutdown();
            };

        };
    };
};
