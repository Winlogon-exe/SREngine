print("IssueManager.py script is running...")

import os
import re

try:
    from github import Github
    from github import Auth
except ImportError:
    subprocess.run([sys.executable, 'pip', 'install', 'pygithub'])
    subprocess.run([sys.executable, '-m', 'pip', 'install', 'pygithub'])
    from github import Github
    from github import Auth

def getIssueBranch(issue):
    body = issue.body
    if body == None:
        return None

    regex = re.search('Commit Branch: `(.*)`', body)
    if regex != None:
        return regex.group(1)
    return None

linuxBuildState = os.getenv('linuxBuildState')
windowsBuildState = os.getenv('windowsBuildState')
commitAuthor = os.getenv('commitAuthor')
commitMessage = os.getenv('commitMessage')
commitBranch = os.getenv('commitBranch')
commitSha = os.getenv('commitSha')
token = os.getenv('ciToken')

try:
    print("Trying to login to GitHub and get the repository")
    auth = Auth.Token(token)
    g = Github(auth=auth)
    repo = g.get_repo("SpaRcle-Studio/SREngine")
except Exception as e:
    print(f"GitHub login exception occurred: {str(e)}")
    exit(0)

issues = repo.get_issues(state='open', labels=['build-failed'])
buildFailed = False

existingIssueComment = "CI: The build in this branch is still failing.\n"
existingIssueComment += f"Linux Build State: `{linuxBuildState}`.\n"
existingIssueComment += f"Windows Build State: `{windowsBuildState}`.\n"
existingIssueComment += f"Commit {commitSha} by @{commitAuthor}.\n"

newIssueBody = "This issue is created automatically by CI.\n\n"
newIssueBody += f"Linux Build State: `{linuxBuildState}`\n"
newIssueBody += f"Windows Build State: `{windowsBuildState}`\n"
newIssueBody += f"Commit: {commitSha}\n"
newIssueBody += f"Commit Branch: `{commitBranch}`\n"
newIssueBody += f"Commit Author: @{commitAuthor}."

if linuxBuildState != 'success' or windowsBuildState != 'success':
    buildFailed = True

issueExists = False

for issue in issues:
    if issue.state == 'closed':
        continue

    issueBranch = getIssueBranch(issue)

    if issueBranch == None:
        print(f"Branch information is not found for issue: {issue.number}")
        continue

    if issueBranch == commitBranch:
        issueExists = True
        if buildFailed:
            issue.create_comment(existingIssueComment)
        else:
            try:
                print(f"Creating comment and closing issue: {issue.number}")
                issue.create_comment(f"Auto-closing as build in this branch is successful now. Commit: {commitSha}")
                issue.edit(state='closed')
                print(f"Success for issue: {issue.number}")
            except Exception as e:
                print(f"Issue managing exception occurred: {str(e)}")

if not issueExists and buildFailed:
    try:
        print("Creating new issue for build failure")
        repo.create_issue(title=f"(Build) Fail on {commitBranch} branch.", body=newIssueBody, labels=['build-failed'])
        print("Issue created successfully")
    except Exception as e:
        print(f"New issue creation exception occurred: {str(e)}")

issueIds = []

try:
    regexClose = re.search('#SR_CLOSE(.*)', commitMessage)
    if regexClose != None:
        closeMacros = regexClose.group(1)
        closeMacros = closeMacros[1:][:-1]
        closeMacros = closeMacros.replace(' ', '')
        issueIds = closeMacros.split(",")
        print(f"Issue Ids are: {issueIds}")
except Exception as e:
    print(f"Regex error occurred: {str(e)}")

for issueId in issueIds:
    try:
        print(f"Trying to get issue: {issueId}")
        issue = repo.get_issue(int(issueId))
        print(f"Creating comment and closing issue: {issueId}")
        issue.create_comment(f"Issue is closed automatically by CI. Commit: {commitSha}")
        issue.edit(state='closed')
        print(f"Success for issue: {issueId}")
    except Exception as e:
        print(f"Issue managing exception occurred: {str(e)}")

print(f"Script execution is finished.")